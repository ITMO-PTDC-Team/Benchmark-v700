#include <verlib/verlib.h>
//#include "rebalance.h"

#ifdef BALANCED
bool balanced = true;
#else
bool balanced = false;
#endif

template <typename K_,
	  typename V_,
	  typename Compare = std::less<K_>>
struct ordered_map {
  using K = K_;
  using V = V_;
  
  static constexpr auto less = Compare{};
  static bool equal(const K& a, const K& b) {
    return !less(a,b) && !less(b,a);
  }

  struct node;
  node* root;

  struct KV {
    K key;
    V value;
    KV(K key, V value) : key(key), value(value) {}
    KV() {}
  };
  static constexpr int block_bytes = 64 * 4; // 6 cache lines
  static constexpr int block_size = (block_bytes - 16)/sizeof(KV) - 1; 

  struct head : verlib::versioned {
    bool is_leaf;
    bool is_sentinal; // only used by leaf
    flck::atomic_write_once<bool> removed; // only used by node
    head(bool is_leaf)
      : is_leaf(is_leaf), is_sentinal(false), removed(false) {}
  };

  // mutable internal node
  struct node : head, flck::lock {
    K key;
    verlib::versioned_ptr<node> left;
    verlib::versioned_ptr<node> right;
    node(K k, node* left, node* right)
      : key(k), head{false}, left(left), right(right) {};
    node(node* left) // root node
      : head{false}, left(left), right(nullptr) {};
  };

  // immutable leaf
  struct leaf : head {
    long size;
    KV keyvals[block_size+1];
    std::optional<V> find(const K& k) {
      for (int i=0; i < size; i++) 
	if (!less(k, keyvals[i].key) && !less(keyvals[i].key, k))
	  return keyvals[i].value;
      return {};
    }
    leaf() : size(0), head{true} {};
    leaf(const KV& keyval) : size(1), head{true} {
      keyvals[0] = keyval;}
  };

  static verlib::memory_pool<node> node_pool;
  static verlib::memory_pool<leaf> leaf_pool;

  //Rebalance<Set<K,V,Compare>> balance;
  //Set() : balance(this) {}

  enum direction {left, right};
  
  static auto find_location(node* root, const K& k) {
    node* gp = nullptr;
    bool gp_left = false;
    node* p = root;
    bool p_left = true;
    node* l = (p->left).read();
    while (!l->is_leaf) {
      gp = p;
      gp_left = p_left;
      p = l;
      p_left = less(k, p->key);
      l = p_left ? (p->left).read() : (p->right).read();
    }
    return std::make_tuple(gp, gp_left, p, p_left, l);
  }

  // inserts a key into a leaf.  If a leaf overflows (> block_size) then
  // the leaf is split in the middle and parent internal node is created
  // to point to the two leaves.  The code within the locks is
  // idempotent.  All changeable values (left and right) are accessed
  // via an atomic_ptr and the pool allocators, which are idempotent.
  // The other values are "immutable" (i.e. they are either written once
  // without being read, or just read).
  bool insert(const K& k, const V& v) {
    return verlib::with_epoch([=] {
      while (true) {
	auto [gp, gp_left, p, p_left, l] = find_location(root, k);
	leaf* old_l = (leaf*) l;
	if (old_l->find(k).has_value()) return false; // already there
	if (p->try_lock([=] {
	      auto ptr = (p_left) ? &(p->left) : &(p->right);

	      // if p has been removed, or l has changed, then exit
	      if (p->removed.load() || ptr->load() != l) return false;
	      // if the leaf is the left sentinal then create new node
	      if (old_l->is_sentinal) {
		leaf* new_l = leaf_pool.new_obj(KV(k,v));
		(*ptr) = node_pool.new_obj(k, l, (node*) new_l);
		return true;
	      }

	      leaf* new_l = leaf_pool.new_init([=] (leaf* new_l) {
		  // first insert into a new block
		  int i=0;
		  for (;i < old_l->size && less(old_l->keyvals[i].key, k); i++) {
		    new_l->keyvals[i] = old_l->keyvals[i];
		  }
		  int offset = 0;
		  if (i == old_l->size || less(k, old_l->keyvals[i].key)) {
		    new_l->keyvals[i] = KV(k,v);
		    offset = 1;
		  }
		  if (offset == 0) {std::cout << "ouch" << std::endl; abort();}
		  for (; i < old_l->size ; i++ )
		    new_l->keyvals[i+offset] = old_l->keyvals[i];
		  new_l->size = old_l->size + offset;
		});

	      // if the block overlflows, split into two blocks and
	      // create a parent
	      if (new_l->size > block_size) {
		int size_l = new_l->size/2;
		int size_r = new_l->size - size_l;
		leaf* new_ll = leaf_pool.new_init([=] (leaf* new_ll) {
		    for (int i =0 ; i < size_l; i++)
		      new_ll->keyvals[i] = new_l->keyvals[i];
		    new_ll->size = size_l;
		  });
		leaf* new_lr = leaf_pool.new_init([=] (leaf* new_lr) {
		    for (int i =0 ; i < size_r; i++)
		      new_lr->keyvals[i] = new_l->keyvals[i + size_l];
		    new_lr->size = size_r;
		  });
		(*ptr) = node_pool.new_obj(new_l->keyvals[size_l].key,
					   (node*) new_ll, (node*) new_lr);
		leaf_pool.retire(new_l);
	      } else (*ptr) = (node*) new_l;

	      // retire the old block
	      leaf_pool.retire(old_l);
	      return true;
	    })) {
	  //if (balanced) balance.rebalance(p, root, k);
	  return true;
	}
	// try again if unsuccessful
      }
    });
  }
  bool insert_(const K& k, const V& v) {return false;} // nyi

  // Removes a key from the leaf.  If the leaf will become empty by
  // removing it, then both the leaf and its parent need to be deleted.
  bool remove(const K& k) {
    return verlib::with_epoch([=] {
      while (true) {
	auto [gp, gp_left, p, p_left, l] = find_location(root, k);
	leaf* old_l = (leaf*) l;
	if (!old_l->find(k).has_value()) return false; // not there
	// The leaf has at least 2 keys, so the key can be removed from the leaf
	if (old_l->size > 1) {
	  if (p->try_lock([=] {
		auto ptr = p_left ? &(p->left) : &(p->right);
		if (p->removed.load() || ptr->load() != l) return false;
		leaf* new_l = leaf_pool.new_init([=] (leaf* new_l) {
		    // copy into new node while deleting k, if there
		    int i = 0;
		    for (;i < old_l->size && old_l->keyvals[i].key < k; i++)
		      new_l->keyvals[i] = old_l->keyvals[i];
		    int offset = equal(old_l->keyvals[i].key, k) ? 1 : 0;
		    for (; i+offset < old_l->size ; i++ )
		      new_l->keyvals[i] = old_l->keyvals[i+offset];
		    new_l->size = i;
		  });

		// update parent to point to new leaf, and retire old
		(*ptr) = (node*) new_l;
		leaf_pool.retire(old_l);
		return true;
	      }))
	    return true;

	  // The leaf has 1 key.  
	} else if (equal(old_l->keyvals[0].key, k)) { // check the one key matches k

	  // We need to delete the leaf (l) and its parent (p), and point
	  // the granparent (gp) to the other child of p.
	  if (gp->try_lock([=] {
	      auto ptr = gp_left ? &(gp->left) : &(gp->right);

	      // if p has been removed, or l has changed, then exit
	      if (gp->removed.load() || ptr->load() != p) return false;

	      // lock p and remove p and l
	      return p->try_lock([=] {
		  node* ll = (p->left).load();
		  node* lr = (p->right).load();
		  if (p_left) std::swap(ll,lr);
		  if (lr != l) return false;
		  p->removed = true;
		  (*ptr) = ll; // shortcut
		  node_pool.retire(p);
		  leaf_pool.retire((leaf*) l);
		  return true; });}))
	    return true;
	} else return true;
	// try again if unsuccessful
      }
    });
  }

  bool remove_(const K& k) { return false;} // nyi

  std::optional<V> find_(const K& k) {
    auto [gp, gp_left, p, p_left, l] = find_location(root, k);
    ((p_left) ? &(p->left) : &(p->right))->validate();
    leaf* ll = (leaf*) l;
    for (int i=0; i < ll->size; i++) 
      if (equal(ll->keyvals[i].key, k)) return ll->keyvals[i].value;
    return {};
  }

  std::optional<V> find(const K& k) {
    return verlib::with_epoch([&] { return find_(k);});
  }
  
  node* empty() {
    leaf* l = leaf_pool.new_obj();
    l->size = 0;
    l->is_sentinal = true;
    return node_pool.new_obj((node*) l);
  }

  ordered_map() : root(empty()) {}
  ordered_map(size_t n) : root(empty()) {}
  ~ordered_map() { retire(root);}

  static void retire(node* p) {
    return;
    if (p == nullptr) return;
    if (p->is_leaf) leaf_pool.retire((leaf*) p);
    else {
      parlay::par_do([&] () { retire((p->left).load()); },
		     [&] () { retire((p->right).load()); });
      node_pool.retire(p);
    }
  }

  double total_height() {
    std::function<size_t(node*, size_t)> hrec;
    hrec = [&] (node* p, size_t depth) {
	     if (p->is_leaf) return depth * ((leaf*) p)->size;
	     size_t d1, d2;
	     parlay::par_do([&] () { d1 = hrec((p->left).load(), depth + 1);},
			    [&] () { d2 = hrec((p->right).load(), depth + 1);});
	     return d1 + d2;
	   };
    return hrec(root->left.load(), 1);
  }
  
  long check() {
    using rtup = std::tuple<K,K,long>;
    std::function<rtup(node*)> crec;
    bool bad_val = false;
    crec = [&] (node* p) {
	     if (p->is_leaf) {
	       leaf* l = (leaf*) p;
	       K minv = l->keyvals[0].key;
	       K maxv = l->keyvals[0].key;
	       for (int i=1; i < l->size; i++) {
		 if (less(l->keyvals[i].key, minv)) minv = l->keyvals[i].key;
		 if (less(maxv, l->keyvals[i].key)) maxv = l->keyvals[i].key;
	       }
	       return rtup(minv, maxv, l->size);
	     }
	     node* l = p->left.load();
	     node* r = p->right.load();
	     K lmin,lmax,rmin,rmax;
	     long lsum,rsum;
	     parlay::par_do([&] () { std::tie(lmin,lmax,lsum) = crec(l);},
			    [&] () { std::tie(rmin,rmax,rsum) = crec(r);});
	     if ((lsum > 0 && !less(lmax, p->key)) || less(rmin, p->key)) {
	       std::cout << "bad value: " << lmax << ", " << p->key << ", " << rmin << std::endl;
	       bad_val = true;
	     }
	     //if (balanced) balance.check_balance(p, l, r);
	     if (lsum == 0) return rtup(p->key, rmax, rsum);
	     else return rtup(lmin, rmax, lsum + rsum);
	   };
    auto [minv, maxv, cnt] = crec(root->left.load());
    // std::cout << "average height = " << ((double) total_height(p) / cnt) << std::endl;
    return bad_val ? -1 : cnt;
  }

  void print() {
    std::function<void(node*)> prec;
    prec = [&] (node* p) {
	     if (p->is_leaf) {
	       leaf* l = (leaf*) p;
	       for (int i=0; i < l->size; i++) 
		 std::cout << l->keyvals[i].key << ", ";
	     } else {
	       prec((p->left).load());
	       prec((p->right).load());
	     }
	   };
    prec(root->left.load());
    std::cout << std::endl;
  }

  static void clear() {
    node_pool.clear();
    leaf_pool.clear();
  }

  static void reserve(size_t n) {
    node_pool.reserve(n/8);
    leaf_pool.reserve(n);
  }

  static void shuffle(size_t n) {
    node_pool.shuffle(n/8);
    leaf_pool.shuffle(n);
  }

  static void stats() {
    node_pool.stats();
    leaf_pool.stats();
  }

};

template <typename K, typename V, typename C>
verlib::memory_pool<typename ordered_map<K,V,C>::node> ordered_map<K,V,C>::node_pool;

template <typename K, typename V, typename C>
verlib::memory_pool<typename ordered_map<K,V,C>::leaf> ordered_map<K,V,C>::leaf_pool;
