#include <verlib/verlib.h>
#include <parlay/primitives.h>

template <typename K>
struct int_string {
  static int length(const K& key) { return sizeof(K);}
  static int get_byte(const K& key, int pos) {
    return (key >> (8*(sizeof(K)-1-pos))) & 255;
  }
};

template <typename K,
          typename V,
          typename String = int_string<K>>
struct ordered_map {

  enum node_type : char {Full, Indirect, Sparse, Leaf};

  constexpr static int max_indirect_size = 64;
  constexpr static int max_sparse_size = 16;
  static constexpr int max_small_leaf_size = 2;
  static constexpr int max_big_leaf_size = 14;
  
  struct header : verlib::versioned {
    const node_type nt;
    char size;
    // every node has a byte position in the key
    // e.g. the root has byte_num = 0
    short int byte_num; 
    K key;
    bool is_leaf() {return nt == Leaf;}
    header(node_type nt, char size) : nt(nt), size(size) {}
    header(const K& key, node_type nt, char size, int byte_num)
      : key(key), nt(nt), size(size), byte_num((short int) byte_num) {}
  };

  // generic node
  struct node : header {
    verlib::atomic_bool removed;
    verlib::lock lck;
    node(node_type nt, char size)
      : header(nt, size), removed(false) {}
    node(const K& key, node_type nt, char size, int byte_num)
      : header(key, nt, size, byte_num), removed(false) {}
  };

  node* root;
  
  using node_ptr = verlib::versioned_ptr<node>;

  // 256 entries, one for each value of a byte, null if empty
  struct full_node : node {
    node_ptr children[256];

    bool is_full() {return false;}

    node_ptr* get_child(const K& k) {
      auto b = String::get_byte(k, header::byte_num);
      return &children[b];}

    void init_child(int k, node* c) {
      auto b = String::get_byte(k, header::byte_num);
      children[b].init(c);
    }

    full_node() : node(Full, 0) {}
  };

  // Up to max_indirect_size entries, with array of 256 1-byte
  // pointers to the entries.  Adding a new child requires a copy.
  // Updating an existing child is done in place.
  struct indirect_node : node {
    char idx[256];
    node_ptr ptr[max_indirect_size];

    bool is_full() {return node::size == max_indirect_size;}
    
    node_ptr* get_child(const K& k) {
      int i = idx[String::get_byte(k, header::byte_num)];
      if (i == -1) return nullptr;
      else return &ptr[i];}

    void init_child(const K& k, node* c) {
      int i = node::size -1;
      idx[String::get_byte(k, header::byte_num)] = i;
      ptr[i].init(c);
    }

    // an empty indirect node
    indirect_node() : node(Indirect, 0) {};
  };

  // Up to max_sparse_size entries each consisting of a key and
  // pointer.  The keys are immutable, but the pointers can be
  // changed.  i.e. Adding a new child requires copying, but updating
  // a child can be done in place.
  struct alignas(64) sparse_node : node {
    unsigned char keys[max_sparse_size];
    node_ptr ptr[max_sparse_size];

    bool is_full() {return node::size == max_sparse_size;}

    node_ptr* get_child(const K& k) {
      __builtin_prefetch (((char*) ptr) + 64);
      int kb = String::get_byte(k, header::byte_num);
      for (int i=0; i < node::size; i++) 
        if (keys[i] == kb) return &ptr[i];
      return nullptr;
    }

    void init_child(const K& k, node* c) {
      int kb = String::get_byte(k, header::byte_num);
      keys[node::size-1] = kb;
      ptr[node::size-1].init(c);
    }

    // constructor for a new sparse node with two children
    sparse_node(int byte_num, node* v1, const K& k1, node* v2, const K& k2)
      : node(k1, Sparse, 2, byte_num) {
      // if (k1 == k2) {
      //         std::cout << "new sparse: " << k1 << ", " << k2 << ", " << byte_num << std::endl;
      //         abort();
      // }
      keys[0] = String::get_byte(k1, byte_num);
      ptr[0].init(v1);
      keys[1] = String::get_byte(k2, byte_num);
      ptr[1].init(v2);
    }

    sparse_node(int byte_num, node** start, node** end) :
      node((*start)->key, Sparse, end-start, byte_num) {
      for (int i=0; i < (end-start); i++) {
        keys[i] = String::get_byte((*(start+i))->key, byte_num);
        ptr[i] = *(start + i);
      }
    }
    
    // an empty sparse node
    sparse_node() : node(Sparse,0) {}
  };

  struct KV {K key; V value;};

  template <int MaxSize>
  struct alignas(64) generic_leaf : header {
    using leaf_ptr = generic_leaf<0>*;
    KV key_vals[MaxSize];

    // starts comparing a and b at the start byte (prior bytes are equal)
    // -1 if a < b, 0 if equal and 1 if a > b
    static int cmp(const K& a, const K& b, int start_byte) {
      int la = String::length(a);
      int lb = String::length(b);
      int l = std::min(la,lb);
      int i = start_byte;
      while (i < l && String::get_byte(a, i) == String::get_byte(b, i)) i++;
      if (i == l) return (la < lb) ? -1 : ((la == lb) ? 0 : 1);
      return (String::get_byte(a,i) < String::get_byte(b,i)) ? -1 : 1;
    } 

    std::optional<V> find(const K& k, int byte_pos) {
      // if integer than just search leaf using <.
      if constexpr (std::is_integral_v<K>) {
        for (int i = 0; i < header::size; i++) {
            if (!(key_vals[i].key < k))
            if (k < key_vals[i].key) return {};
            else return key_vals[i].value;
        }
      } else {
        if (byte_pos < header::byte_num) return {};
        for (int i = 0; i < header::size; i++) {
            int x = cmp(key_vals[i].key, k, byte_pos);
          if (x != -1)
            if (x == 1) return {};
            else return key_vals[i].value;
        }
      }
      return {};}

    static int first_diff(int byte_pos, KV* start, KV* end) {
      if (end-start == 1) return String::length(start->key);
      int j = byte_pos;
      while (true) {
        int byteval = String::get_byte(start->key, j);
        for (KV* ptr = start + 1; ptr < end; ptr++) {
          if (j >= String::length(ptr->key) ||
              String::get_byte(ptr->key, j) != byteval) return j;
        }
        j++;
      }
    }

    // create singleton leaf
    generic_leaf(K& key, V& value) : header(key, Leaf, 1, String::length(key)) {
      key_vals[0] = KV{key, value};
    }

    // create multi leaf
    generic_leaf(int byte_pos, KV* start, KV* end)
      : header(start->key, Leaf, end-start, first_diff(byte_pos, start, end)) {
      for (int i=0; i < (end-start); i++) key_vals[i] = start[i];
    }

    static void insert(KV* in, KV* out, int n, const K& key, const V& value) {
      int i = 0;
      while (i < n && in[i].key < key) {
        out[i] = in[i];
        i++;
      }
      out[i] = KV{key,value};
      while (i < n) {
              out[i+1] = in[i];
        i++;
      }
    }
      
    // add to leaf
    generic_leaf(int byte_pos, leaf_ptr l, K& key, V& value) 
      : header(l->key, Leaf, l->size + 1, byte_pos) {
      insert(l->key_vals, key_vals, l->size, key, value);
      // for (int i = 1; i < header::size; i++)
      //         if (!(key_vals[i-1].key < key_vals[i].key)) {
      //           std::cout << "x out of order at leaf: " << i << ", " << header::size << ", " << std::hex << 
      //             key_vals[i-1].key << ", " << key_vals[i].key << std::dec << std::endl;
      //           abort();
      //         }
    }

    // remove from leaf
    generic_leaf(leaf_ptr l, K& key) 
      : header(l->key, Leaf, l->size - 1, 0) {
      int j = 0;
      for (int i=0; i < l->size; i++) {
        if (l->key_vals[i].key != key) 
          key_vals[j++] = l->key_vals[i];
      }
      header::key = key_vals[0].key;
      header::byte_num = first_diff(l->byte_num, &key_vals[0], &key_vals[header::size]);
    }

  };
  using leaf = generic_leaf<0>; // only used generically
  using small_leaf = generic_leaf<max_small_leaf_size>;
  using big_leaf = generic_leaf<max_big_leaf_size>;
  
  static verlib::memory_pool<full_node> full_pool;
  static verlib::memory_pool<indirect_node> indirect_pool;
  static verlib::memory_pool<sparse_node> sparse_pool;
  static verlib::memory_pool<small_leaf> small_leaf_pool;
  static verlib::memory_pool<big_leaf> big_leaf_pool;

  // dispatch based on node type
  // A returned nullptr means no child matching the key
  static inline node_ptr* get_child(node* x, const K& k) {
    switch (x->nt) {
    case Full : return ((full_node*) x)->get_child(k);
    case Indirect : return ((indirect_node*) x)->get_child(k);
    case Sparse : return ((sparse_node*) x)->get_child(k);
    }
    return nullptr;
  }

  // dispatch based on node type
  static inline bool is_full(node* p) {
    switch (p->nt) {
    case Full : return ((full_node*) p)->is_full(); // never full
    case Indirect : return ((indirect_node*) p)->is_full();
    case Sparse : return ((sparse_node*) p)->is_full();
    }
    return false;
  }

  // Adds a new child to p with key k and value v
  // gp is p's parent (i.e. grandparent)
  // This involve copying p and updating gp to point to it.
  // This should never be called on a full node.
  // Returns false if it fails.
  static bool add_child(node* gp, node* p, const K& k, const V& v) {
    return gp->lck.try_lock([=] {
        auto child_ptr = get_child(gp, p->key);
        bool rm = gp->removed.load();
        bool pt = child_ptr->load() != p;
        if (gp->removed.load() || child_ptr->load() != p)
          return false;
        return p->lck.try_lock([=] {
            if (get_child(p,k) != nullptr) return false;
            node* c = (node*) small_leaf_pool.new_obj(k, v);
            if (p->nt == Indirect) {
              indirect_node* i_n = (indirect_node*) p;
              i_n->removed = true; 
              if (is_full(p)) {
                // copy indirect to full
                *child_ptr = (node*) full_pool.new_init([=] (full_node* f_n) {
                   f_n->key = i_n->key;
                  f_n->byte_num = i_n->byte_num;
                  for (int i=0; i < 256; i++) {
                    int j = i_n->idx[i];
                    if (j != -1) f_n->children[i].init(i_n->ptr[j].load());
                  }
                  // seemingly broken g++ (10.3) compiler makes the following fail
                  //   f_n->init_child(k, c);
                  // inlining by hand fixes it.
                  auto b = String::get_byte(k, f_n->byte_num);
                  f_n->children[b].init(c);});
              } else {
                // copy indirect to indirect
                *child_ptr = (node*) indirect_pool.new_init([=] (indirect_node* i_c) {
                    i_c->key = i_n->key;
                  i_c->byte_num = i_n->byte_num;
                  i_c->size = i_n->size + 1;
                  for (int i=0; i < 256; i++)
                    i_c->idx[i] = i_n->idx[i];
                  for (int i=0; i < i_n->size; i++) 
                    i_c->ptr[i].init(i_n->ptr[i].load());
                  i_c->init_child(k, c);});
                }
                indirect_pool.retire(i_n);
              } else { // (p->nt == Sparse)
                sparse_node* s_n = (sparse_node*) p;
                s_n->removed = true;
                if (is_full(p)) {
                  // copy sparse to indirect
                  *child_ptr = (node*) indirect_pool.new_init([=] (indirect_node* i_n) {
                      i_n->key = s_n->key;
                    i_n->byte_num = s_n->byte_num;
                    i_n->size = max_sparse_size + 1;
                    for (int i=0; i < 256; i++) i_n->idx[i] = -1;
                    for (int i=0; i < max_sparse_size; i++) {
                      i_n->idx[s_n->keys[i]] = i; 
                      i_n->ptr[i].init(s_n->ptr[i].load());
                    }
                    i_n->init_child(k, c);
                  });  
                } else {
                  // copy sparse to sparse
                  *child_ptr = (node*) sparse_pool.new_init([=] (sparse_node* s_c) {
                      s_c->key = s_n->key;
                    s_c->byte_num = s_n->byte_num;
                    s_c->size = s_n->size + 1;
                    for (int i=0; i < s_n->size; i++) {
                      s_c->keys[i] = s_n->keys[i];
                      s_c->ptr[i].init(s_n->ptr[i].load());
                    }
                    s_c->init_child(k, c);});
                }
                sparse_pool.retire(s_n);
            }
            return true;}); // end try_lock(p->lck
        return true;}); // end try_lock(gp->lck
  }

  //*** will return one of 4 things: no child, empty child, leaf, subtree (cut edge)
  static auto find_location(node* root, const K& k) {
    int byte_pos = 0;
    node* gp = nullptr;
    node* p = root;
    while (true) {
      node_ptr* cptr = get_child(p, k);
      if (cptr == nullptr) // has no child with given key
        return std::make_tuple(gp, p, cptr, (node*) nullptr, byte_pos);
      // could be read()
      node* c = cptr->load();
      if (c == nullptr) // has empty child with given key
        return std::make_tuple(gp, p, cptr, c, byte_pos);

      byte_pos++;
      while (byte_pos < c->byte_num &&
             String::get_byte(k, byte_pos) == String::get_byte(c->key, byte_pos))
        byte_pos++;

      if (byte_pos != c->byte_num || c->is_leaf())
        return std::make_tuple(gp, p, cptr, c, byte_pos);

      gp = p;
      p = c;
    }
  }

  static node* new_leaf(int byte_pos, KV* start, KV* end) {
    if ((end-start) > max_small_leaf_size)
      return (node*) big_leaf_pool.new_obj(byte_pos, start, end);
    else return (node*) small_leaf_pool.new_obj(byte_pos, start, end);
  }
  
  bool insert_(const K& k, const V& v) {
    return flck::try_loop([&] () {return try_insert(k, v);});}

  bool insert(const K& k, const V& v) {
    return verlib::with_epoch([=] { return insert_(k, v);});}
  
  std::optional<bool> try_insert(const K& k, const V& v, bool upsert=false) {
    auto [gp, p, cptr, c, byte_pos] = find_location(root, k);
    //std::cout << byte_pos << std::endl;
    if (c != nullptr && c->is_leaf() &&
        ((leaf*) c)->find(k, byte_pos).has_value()) // already in the tree
      if (p->lck.read_lock([&] {return (cptr->load() == c) && !p->removed.load();}))
        return false;
      else return {};

    if (cptr != nullptr) {// child pointer exists, always true for full node
      if (p->lck.try_lock([=] {
           // exit and retry if state has changed
          if (p->removed.load() || cptr->load() != c) return false;

          // fill a null pointer with the new leaf
          if (c == nullptr)
            (*cptr) = (node*) small_leaf_pool.new_obj(k, v);
          else if (c->is_leaf()) {
            leaf* l = (leaf*) c;
            small_leaf* sl = (small_leaf*) c;
            big_leaf* bl = (big_leaf*) c;
            if (l->size < max_small_leaf_size) {
              *cptr = (node*) small_leaf_pool.new_obj(byte_pos, l, k, v);
              small_leaf_pool.retire(sl);
            } else if (l->size == max_small_leaf_size) {
              *cptr = (node*) big_leaf_pool.new_obj(byte_pos, l, k, v);
              small_leaf_pool.retire(sl);
            } else if (l->size < max_big_leaf_size) {
              *cptr = (node*) big_leaf_pool.new_obj(byte_pos, l, k, v);
              big_leaf_pool.retire(bl);
            } else { // too large
              int n = max_big_leaf_size + 1;

              // insert new key-value pair into l and put result into tmp
              KV tmp[n];
              leaf::insert(l->key_vals, tmp, n - 1, k, v);
              // for (int i=0; i < max_big_leaf_size; i++)
              //          tmp[i] = l->key_vals[i];
              // tmp[max_big_leaf_size] = KV{k,v};
              // std::sort(tmp, tmp + n, [&] (KV& a, KV& b) {return a.key < b.key;});

              // break tmp up into multiple leafs base on byte value at byte_pos
              auto gb = [&] (const KV& kv) {return String::get_byte(kv.key, byte_pos);};
              int j = 0;
              int start = 0;
              int byteval = gb(tmp[0]);
              node* children[n];
              for (int i=1; i < n; i++) {
                if (gb(tmp[i]) != byteval) {
                  children[j++] = new_leaf(byte_pos+1, &tmp[start], &tmp[i]);
                  start = i;
                  byteval = gb(tmp[i]);
                }
              }
              children[j++] = new_leaf(byte_pos+1, &tmp[start], &tmp[n]);

              // insert the new leaves into a sparse node
              *cptr = (node*) sparse_pool.new_obj(byte_pos, &children[0], &children[j]);
              big_leaf_pool.retire(bl);
            }
          } else { // not a leaf
            node* new_l = (node*) small_leaf_pool.new_obj(k, v);
            *cptr = (node*) sparse_pool.new_obj(byte_pos, c, c->key,
                                                new_l, k);
          }
          return true;})) return true;
    } else { // no child pointer, need to add
      bool x = add_child(gp, p, k, v);
      if (x) return true;
    }
    return {};
  }

  // returns other child if node is sparse and has two children, one
  // of which is c, otherwise returns nullptr
  static node* single_other_child(node* p, node* c) {
    if (p->nt != Sparse) return nullptr;
    sparse_node* ps = (sparse_node*) p;
    node* result = nullptr;
    for (int i=0; i < ps->size; i++) {
      node* oc = ps->ptr[i].load();
      if (oc != nullptr && oc != c)
        if (result != nullptr) return nullptr; // quit if second child
        else result = oc; // set first child
    }
    return result;
  }

  bool remove_(const K& k) {
    return flck::try_loop([&] () {return try_remove(k);});}

  bool remove(const K& k) {
    return verlib::with_epoch([=] { return remove_(k);});}

  // currently a "lazy" remove that only removes
  //   1) the leaf
  //   2) its parent if it is sparse with just two children
  std::optional<bool> try_remove(const K& k) {
    auto [gp, p, cptr, c, byte_pos] = find_location(root, k);
    // if not found return
    if (c == nullptr || !(c->is_leaf() && ((leaf*) c)->find(k, byte_pos).has_value()))
      if (p->lck.read_lock([&] {return (cptr->load() == c) && !p->removed.load();}))
        return false;
      else return {};
    if (p->lck.try_lock([=] {
        if (p->removed.load() || cptr->load() != c) return false;
        leaf* l = (leaf*) c;
        if (l->size == 1) {
          node* other_child = single_other_child(p, c);
          // if (other_child != nullptr && gp->nt != Sparse) {
          //   // if parent will become singleton try to remove parent as well
          //   return gp->lck.try_lock([=] {
          //     auto child_ptr = get_child(gp, p->key);
          //     if (gp->removed.load() || child_ptr->load() != p)
                // return false;
          //     *child_ptr = other_child;
          //     p->removed = true;
          //     sparse_pool.retire((sparse_node*) p);
          //     small_leaf_pool.retire((small_leaf*) l);
          //     return true;});
          // } else 
    { // just remove child
            *cptr = nullptr;
            small_leaf_pool.retire((small_leaf*) l);
            return true;
          }
        } else { // at least 2 in leaf
          if (l->size > max_small_leaf_size + 1) {
            *cptr = (node*) big_leaf_pool.new_obj(l, k);
            big_leaf_pool.retire((big_leaf*) l);
          } else if (l->size == max_small_leaf_size + 1) {
            *cptr = (node*) small_leaf_pool.new_obj(l, k);
            big_leaf_pool.retire((big_leaf*) l);
          } else {
            *cptr = (node*) small_leaf_pool.new_obj(l, k);
            small_leaf_pool.retire((small_leaf*) l);
          }
          return true;
        }})) return true;
    return {};
  }

  std::optional<std::optional<V>> try_find(const K& k) {
    using ot = std::optional<std::optional<V>>;
    auto [gp, p, cptr, l, pos] = find_location(root, k);
    if (p->lck.read_lock([&] {return (cptr->load() == l) && !p->removed.load();}))
      if (l == nullptr) return ot(std::optional<V>());
      else return ot(((leaf*) l)->find(k, pos));
    else return ot();
  }

  std::optional<V> find_(const K& k) {
    auto [gp, p, cptr, l, pos] = find_location(root, k);
    auto ll = (leaf*) l;
    if (l == nullptr) return {};
    else return ((leaf*) l)->find(k, pos);
  }

  std::optional<V> find(const K& k) {
    return verlib::with_epoch([&] {return find_(k);}); }

  std::optional<V> find_locked(const K& k) {
    return flck::try_loop([&] {return try_find(k);}); }

  template<typename AddF>
  static void range_internal(node* a, AddF& add,
                      std::optional<K> start, std::optional<K> end, int pos) {
    if (a == nullptr) return;
    std::optional<K> empty;
    if (a->nt == Leaf) {
      leaf* l = (leaf*) a;
      int s = 0;
      int e = a->size;
      if (start.has_value())
        while (l->key_vals[s].key < *start) s++;
      if (end.has_value())
        while (l->key_vals[e-1].key > *end) e--;
      for (int i = s; i < e; i++)
        add(l->key_vals[i].key, l->key_vals[i].value);
      return;
    }
    for (int i = pos; i < a->byte_num; i++) {
      if (start == empty && end == empty) break;
      if (start.has_value()
          && String::get_byte(start.value(), i) > String::get_byte(a->key, i)
          || end.has_value()
          && String::get_byte(end.value(), i) < String::get_byte(a->key, i))
        return;
      if (start.has_value() &&
          String::get_byte(start.value(), i) < String::get_byte(a->key,i)) 
        start = empty;
      if (end.has_value() &&
          String::get_byte(end.value(), i) > String::get_byte(a->key, i)) {
        end = empty;
      }
    }
    int sb = start.has_value() ? String::get_byte(start.value(), a->byte_num) : 0;
    int eb = end.has_value() ? String::get_byte(end.value(), a->byte_num) : 255;
    if (a->nt == Full) {
      for (int i = sb; i <= eb; i++) 
        range_internal(((full_node*) a)->children[i].read_snapshot(), add,
                       start, end, a->byte_num);
    } else if (a->nt == Indirect) {
      for (int i = sb; i <= eb; i++) {
        indirect_node* ai = (indirect_node*) a;
        int o = ai->idx[i];
        if (o != -1) {
          range_internal(ai->ptr[o].read_snapshot(), add,
                                    start, end, a->byte_num);
        }
      }
    } else { // Sparse
      sparse_node* as = (sparse_node*) a;
      for (int i = 0; i < as->size; i++) {
        int b = as->keys[i];
        if (b >= sb && b <= eb)
          range_internal(as->ptr[i].read_snapshot(), add, start, end, a->byte_num);
      }
    }
  }                       

  template<typename AddF>
  void range_(AddF& add, const K& start, const K& end) {
    range_internal(root, add,
                   std::optional<K>(start), std::optional<K>(end), 0);
  }

  ordered_map() {
    auto r = full_pool.new_obj();
    r->byte_num = 0;
    root = (node*) r;
  }

  ordered_map(size_t n) {
    auto r = full_pool.new_obj();
    r->byte_num = 0;
    root = (node*) r;
  }

  void print() {
    std::function<void(node*)> prec;
    prec = [&] (node* p) {
             if (p == nullptr) return;
             switch (p->nt) {
             case Leaf : {
               auto l = (leaf*) p;
               std::cout << std::hex << l->key << ":" << l->key_vals[0].key << std::dec << ", ";
               return;
             }
             case Full : {
               auto f_n = (full_node*) p;
               for (int i=0; i < 256; i++) {
                 prec(f_n->children[i].load());
               }
               return;
             }
             case Indirect : {
               auto i_n = (indirect_node*) p;
               for (int i=0; i < 256; i++) {
                 int j = i_n->idx[i];
                 if (j != -1) prec(i_n->ptr[j].load());
               }
               return;
             }
             case Sparse : {
               using pr = std::pair<int,node*>;
               auto s_n = (sparse_node*) p;
               std::vector<pr> v;
               for (int i=0; i < s_n->size; i++)
                 v.push_back(std::make_pair(s_n->keys[i], s_n->ptr[i].load()));
               std::sort(v.begin(), v.end()); 
               for (auto x : v) prec(x.second);
               return;
             }
             }
           };
    prec(root);
    std::cout << std::endl;
  }

  static void retire_recursive(node* p) {
    if (p == nullptr) return;
    if (p->nt == Leaf) {
      if (p->size > max_small_leaf_size)
        big_leaf_pool.retire((big_leaf*) p);
      else small_leaf_pool.retire((small_leaf*) p);
    }
    else if (p->nt == Sparse) {
      auto pp = (sparse_node*) p;
      parlay::parallel_for(0, pp->size, [&] (size_t i) {
          retire_recursive(pp->ptr[i].load());});
      sparse_pool.retire(pp);
    } else if (p->nt == Indirect) {
      auto pp = (indirect_node*) p;
      parlay::parallel_for(0, pp->size, [&] (size_t i) {
          retire_recursive(pp->ptr[i].load());});
      indirect_pool.retire(pp);
    } else {
      auto pp = (full_node*) p;
      parlay::parallel_for(0, 256, [&] (size_t i) {
          retire_recursive(pp->children[i].load());});
      full_pool.retire(pp);
    }
  }
  ~ordered_map() { retire_recursive(root);}
  
  long check() {
    std::function<size_t(node*)> crec;
    crec = [&] (node* p) -> size_t {
             if (p == nullptr) return 0;
             switch (p->nt) {
             case Leaf : {
               leaf* l = (leaf*) p;
               for (int i = 1; i < l->size; i++)
                 if (!(l->key_vals[i-1].key < l->key_vals[i].key)) {
                   std::cout << "out of order at leaf: " << i << ", " << (int) l->size << ", " << std::hex << 
                     l->key_vals[i-1].key << ", " << l->key_vals[i].key << std::dec << std::endl;
                   abort();
                 }
               return l->size;
             }
             case Full : {
               auto f_n = (full_node*) p;
               auto x = parlay::tabulate(256, [&] (size_t i) {
                                                return crec(f_n->children[i].load());});
               return parlay::reduce(x);
             }
             case Indirect : {
               auto i_n = (indirect_node*) p;
               auto x = parlay::tabulate(256, [&] (size_t i) {
                                                int j = i_n->idx[i];
                                                return (j == -1) ? 0 : crec(i_n->ptr[j].load());});
               return parlay::reduce(x);
             }
             case Sparse : {
               auto s_n = (sparse_node*) p;
               auto x = parlay::tabulate(s_n->size,
                                         [&] (size_t i) {return crec(s_n->ptr[i].load());});
               return parlay::reduce(x);
             }
             }
             return 0;
           };
    size_t cnt = crec(root);
    return cnt;
  }

  static void clear() {
    full_pool.clear();
    indirect_pool.clear();
    sparse_pool.clear();
    small_leaf_pool.clear();
    big_leaf_pool.clear();
  }

  static void reserve(size_t n) {}
  
  static void shuffle(size_t n) {
    full_pool.shuffle(n/100);
    indirect_pool.shuffle(n/10);
    sparse_pool.shuffle(n/5);
    small_leaf_pool.shuffle(n);
    big_leaf_pool.shuffle(n);
  }

  static void stats() {
    full_pool.stats();
    indirect_pool.stats();
    sparse_pool.stats();
    small_leaf_pool.stats();
    big_leaf_pool.stats();
  }
  
};

template <typename K, typename V, typename S>
verlib::memory_pool<typename ordered_map<K,V,S>::full_node> ordered_map<K,V,S>::full_pool;
template <typename K, typename V, typename S>
verlib::memory_pool<typename ordered_map<K,V,S>::indirect_node> ordered_map<K,V,S>::indirect_pool;
template <typename K, typename V, typename S>
verlib::memory_pool<typename ordered_map<K,V,S>::sparse_node> ordered_map<K,V,S>::sparse_pool;
template <typename K, typename V, typename S>
verlib::memory_pool<typename ordered_map<K,V,S>::small_leaf> ordered_map<K,V,S>::small_leaf_pool;
template <typename K, typename V, typename S>
verlib::memory_pool<typename ordered_map<K,V,S>::big_leaf> ordered_map<K,V,S>::big_leaf_pool;
