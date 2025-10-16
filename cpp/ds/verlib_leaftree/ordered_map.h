#include <verlib/verlib.h>
#include <parlay/parallel.h>

template <typename K,
	  typename V,
	  typename Compare = std::less<K>>
struct ordered_map {

  struct internal;
  internal* root;
  
  // common header for internal nodes and leaves
  struct node : verlib::versioned {
    K key;
    bool is_leaf;
    bool is_sentinal;
    node(K key, bool is_leaf)
      : key(key), is_leaf(is_leaf), is_sentinal(false) {}
    node(bool is_leaf) :
      is_leaf(is_leaf), is_sentinal(true) {}
  };

  // internal node
  struct internal : node {
    verlib::lock lck;
    verlib::atomic_bool removed;
    verlib::versioned_ptr<node> left;
    verlib::versioned_ptr<node> right;
    internal(K k, node* left, node* right)
      : node{k,false}, left(left), right(right), removed(false) {};
    internal(node* left) // for the root, only has a left pointer
      : node{false}, left(left), right(nullptr), removed(false) {};
  };

  struct leaf : node {
    V value;
    leaf(K k, V v) : node{k,true}, value(v) {};
    leaf() : node{true} {}; // for the sentinal leaf
  };

  static bool Less(const K& k, node* a) {
    return !a->is_sentinal && Compare{}(k, a->key);
  }
  static bool Equal(const K& k, node* a) {
    return !(a->is_sentinal || Compare{}(k, a->key) || Compare{}(a->key ,k));
  }

  static verlib::memory_pool<internal> internal_pool;
  static verlib::memory_pool<leaf> leaf_pool;

  static auto find_location(internal* root, const K& k) {
    internal* gp = nullptr;
    bool gp_left = false;
    internal* p = root;
    bool p_left = true;
    node* l = (p->left).load();
    while (!l->is_leaf) {
      gp = p;
      gp_left = p_left;
      p = (internal*) l;
      p_left = Less(k, p);
      l = p_left ? (p->left).load() : (p->right).load();
    }
    return std::make_tuple(gp, gp_left, p, p_left, (leaf*) l);
  }

  bool insert_(const K& k, const V& v) {
    return flck::try_loop([&] {return try_insert(k, v);});}
  
  bool upsert(const K& k, const V& v) {
    return verlib::with_epoch([=] {
      return flck::try_loop([&] {return try_insert(k, v, true);});
    });
  }

  bool insert(const K& k, const V& v) {
    return verlib::with_epoch([=] {return insert_(k, v);}); }
      
  std::optional<bool> try_insert(const K& k, const V& v, bool upsert=false) {
    auto [gp, gp_left, p, p_left, l] = find_location(root, k);
    auto ptr = p_left ? &(p->left) : &(p->right);
    if (Equal(k, l))
	//(!l->is_sentinal && (!upsert && equal(l->key, k))))
      // || (upsert && prev_leaf != nullptr && equal(prev_leaf->key, k) &&
      // l != prev_leaf)))
      if (p->lck.read_lock([&] {return (ptr->load() == (node*) l) && !p->removed.load();}))
	return false;
      else return {};
    //prev_leaf = l;
    bool r = p->lck.try_lock([=] {
	auto l_new = ptr->load();
	if (p->removed.load() || l_new != l) return false;
	node* new_l = leaf_pool.new_obj(k, v);
	//if (!l->is_sentinal && equal(k, l->key)) *ptr = new_l; // update existing key (only if upsert)
	//else
	*ptr = (Less(k, l) ?
		internal_pool.new_obj(l->key, new_l, l) :
		internal_pool.new_obj(k, l, new_l));
	return true;});
    if (r) return !Equal(k, l); 
    else return {};
  }

  bool remove_(const K& k) {
    return flck::try_loop([&] {	return try_remove(k); });}

  bool remove(const K& k) {
    return verlib::with_epoch([=] { return remove_(k);});}

  std::optional<bool> try_remove(const K& k) {
    auto [gp, gp_left, p, p_left, l] = find_location(root, k);
    auto ptr = gp_left ? &(gp->left) : &(gp->right);
    if (l->is_sentinal) return false; // need to deal with gp == nullptr
    if (!Equal(k, l)) {
      // || (prev_leaf != nullptr && prev_leaf != l))
      if (gp->lck.read_lock([&] {return (ptr->load() == p) && !gp->removed.load();}))
	return false;
      else return {};
    }
    // prev_leaf = l;
    if (gp->lck.try_lock([=] {
      return p->lck.try_lock([=] {
	  if (gp->removed.load() || ptr->load() != p) return false;
	  node* ll = (p->left).load();
	  node* lr = (p->right).load();
	  if (p_left) std::swap(ll,lr);
	  if (lr != l) return false;
	  p->removed = true;
	  (*ptr) = ll; // shortcut
	  internal_pool.retire(p);
	  leaf_pool.retire(l);
	  return true; });}))
      return std::optional<bool>(true);
    else return {};
  }

  std::optional<V> find_(const K& k) {
    auto ptr = &(root->left);
    node* l = ptr->load();
    while (!l->is_leaf) {
      internal* p = (internal*) l;
      auto ptr = Less(k, p) ? &(p->left) : &(p->right);
      l = ptr->load();
    }
    auto ll = (leaf*) l;
    if (Equal(k, ll)) return ll->value; 
    else return {};
  }

  std::optional<V> find(const K& k) {
    return verlib::with_epoch([&] { return find_(k);});}

  std::optional<V> find_locked(const K& k) {
    return flck::try_loop([&] {return try_find(k);});}

  std::optional<std::optional<V>> try_find(const K& k) {
    using ot = std::optional<std::optional<V>>;
    auto [gp, gp_left, p, p_left, l] = find_location(root, k);
    auto ptr = p_left ? &(p->left) : &(p->right);
    if (p->lck.read_lock([&] {return (ptr->load() == (node*) l) && !p->removed.load();}))
      return ot(Equal(k, l) ?
		std::optional<V>(l->value) :
		std::optional<V>());
    return ot();
  }

  ordered_map() : root(internal_pool.new_obj(leaf_pool.new_obj())) {}
  ordered_map(size_t n) : root(internal_pool.new_obj(leaf_pool.new_obj())) {}
  ~ordered_map() { retire(root);}
  
  void print() {
    std::function<void(node*)> prec;
    prec = [&] (node* p) {
	     if (p->is_sentinal) std::cout << "[";
	     else if (p->is_leaf) std::cout << p->key;
	     else {
	       internal* pp = (internal*) p;
	       prec((pp->left).load());
	       if (!(pp->left).load()->is_sentinal)
		 std::cout << ", ";
	       prec((pp->right).load());
	     }
	   };
    prec(root->left.load());
    std::cout << "]" << std::endl;
  }

  static void retire(node* p) {
    if (p == nullptr) return;
    if (p->is_leaf) leaf_pool.retire((leaf*) p);
    else {
      internal* pp = (internal*) p;
      parlay::par_do([&] () { retire((pp->left).load()); },
		     [&] () { retire((pp->right).load()); });
      internal_pool.retire(pp);
    }
  }
  
  // return total height
  double total_height() {
    std::function<size_t(node*, size_t)> hrec;
    hrec = [&] (node* p, size_t depth) {
	     if (p->is_leaf) return depth;
	     internal* pp = (internal*) p;
	     size_t d1, d2;
	     parlay::par_do([&] () { d1 = hrec((pp->left).load(), depth + 1);},
			    [&] () { d2 = hrec((pp->right).load(), depth + 1);});
	     return d1 + d2;
	   };
    return hrec(root->left.load(), 1);
  }

  long check() {
    using rtup = std::tuple<K, K, long>;
    std::function<rtup(node*,bool)> crec;
    crec = [&] (node* p, bool leftmost) {
	     if (p->is_leaf)
	       if (leftmost) {
		 if (p->is_sentinal) return rtup(p->key, p->key, 0);
		 else {
		   std::cout << "error, leftmost leaf is not a sentinal" << std::endl;
		   abort();
		 }
	       } else if (p->is_sentinal) {
		 std::cout << "error, not leftmost leaf is a sentinal" << std::endl;
		 abort();
	       } else return rtup(p->key, p->key, 1);
	     if (p->is_sentinal) {
	       std::cout << "error, interior node is a sentinal" << std::endl;
	       abort();
	     }
	     internal* pp = (internal*) p;
	     K lmin,lmax,rmin,rmax;
	     long lsum, rsum;
	     parlay::par_do([&] () { std::tie(lmin,lmax,lsum) = crec((pp->left).load(), leftmost);},
			    [&] () { std::tie(rmin,rmax,rsum) = crec((pp->right).load(), false);});
	     if ((lsum !=0 && !Less(lmax, pp)) || Less(rmin, pp))
	       std::cout << "out of order key: " << lmax << ", " << pp->key << ", " << rmin << std::endl;
	     if (lsum == 0) return rtup(pp->key, rmax, rsum);
	     else return rtup(lmin, rmax, lsum + rsum);
	   };
    auto l = (root->left).load();
    auto [minv, maxv, cnt] = crec(l, true);
    //std::cout << "average height = " << ((double) total_height(p) / cnt) << std::endl;
    return cnt;
  }
  
  static void clear() {
    internal_pool.clear();
    leaf_pool.clear();
  }

  static void reserve(size_t n) {
    internal_pool.reserve(n);
    leaf_pool.reserve(n);
  }

  static void shuffle(size_t n) {
    internal_pool.shuffle(n);
    leaf_pool.shuffle(n);
  }

  static void stats() {
    internal_pool.stats();
    leaf_pool.stats();
  }
  
};

template <typename K, typename V, typename C>
verlib::memory_pool<typename ordered_map<K,V,C>::internal> ordered_map<K,V,C>::internal_pool;

template <typename K, typename V, typename C>
verlib::memory_pool<typename ordered_map<K,V,C>::leaf> ordered_map<K,V,C>::leaf_pool;
