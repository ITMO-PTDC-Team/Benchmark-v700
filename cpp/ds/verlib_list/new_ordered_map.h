#include <verlib/verlib.h>

template <typename K,
          typename V,
          typename Compare = std::less<K>>
struct ordered_map {
  static constexpr auto less = Compare{};

  struct alignas(32) node : verlib::versioned {
    verlib::versioned_ptr<node> next;
    K key;
    V value;
    bool is_end;
    verlib::atomic_bool removed;
    verlib::lock lck;
    node(K key, V value, node* next)
      : key(key), value(value), next(next), is_end(false), removed(false) {};
    node(node* next, bool is_end) // for head and tail
      : next(next), is_end(is_end), removed(false) {};
#ifdef Recorded_Once
    node(node* n) // copy from pointer
      : key(n->key), value(n->value), next(n->next.load()),
      is_end(n->is_end), removed(false) {}
#endif
  };

  node* root;
  
  static auto find_location(node* root, const K& k) {
    node* cur = root;
    node* nxt = (cur->next).load();
    while (true) {
      node* nxt_nxt = (nxt->next).load(); // prefetch
      if (nxt->is_end || !less(nxt->key, k)) break;
      cur = nxt;
      nxt = nxt_nxt;
    }
    return std::make_pair(cur, nxt);
  }

  std::optional<bool> try_insert(const K& k, const V& v) {
    auto [cur, nxt] = find_location(root, k);
    if (!nxt->is_end && !less(k, nxt->key)) //already there
      if (cur->lck.read_lock([&] {return (cur->next.load() == nxt) && !cur->removed.load();}))
	return false;
      else return {};
    if (cur->lck.try_lock([=] {
      if (!cur->removed.load() && (cur->next).load() == nxt) {
	node* new_node = flck::New<node>(k, v, nxt);
	cur->next = new_node; // splice in
	return true;
      } else return false;})) return true;
    else return {}; // failed
  }

  bool insert_(const K& k, const V& v) {
    return flck::try_loop([&] {return try_insert(k, v);});}
  
  bool insert(const K& k, const V& v) {
    return verlib::with_epoch([=] {return insert_(k, v);}); }

  std::optional<bool> try_remove(const K& k) {
    auto [cur, nxt] = find_location(root, k);
    if (nxt->is_end || less(k, nxt->key)) // not found
      if (cur->lck.read_lock([&] {return (cur->next.load() == nxt) && !cur->removed.load();}))
	return false;
      else return {};
    if (cur->lck.try_lock([=] {
        if (cur->removed.load() || (cur->next).load() != nxt)
	  return false;
	return nxt->lck.try_lock([=] {
        node* nxtnxt = (nxt->next).load();
#ifdef Recorded_Once
	// if recoreded once then need to copy nxtnxt and point to it
	return nxtnxt->lck.try_lock([=] {
          nxt->removed = true;
	  nxtnxt->removed = true;
	  cur->next = flck::New<node>(nxtnxt); // copy nxt->next
	  flck::Retire<node>(nxt);
	  flck::Retire<node>(nxtnxt); 
	  return true;});
#else
	nxt->removed = true;
	cur->next = nxtnxt; // shortcut
	flck::Retire<node>(nxt);
	return true;
#endif
        });}))
      return true;
    else return {};
  }

  bool remove_(const K& k) {
    return flck::try_loop([&] {return try_remove(k);});}
  
  bool remove(const K& k) {
    return verlib::with_epoch([=] { return remove_(k);});}

  std::optional<V> find_locked(const K& k) {
    return flck::try_loop([&] {return try_find(k);});
  }

  std::optional<std::optional<V>> try_find(const K& k) {
    using ot = std::optional<std::optional<V>>;
    auto [cur, nxt] = find_location(root, k);
    if (cur->lck.read_lock([&] {return (cur->next.load() == nxt) && !cur->removed.load();}))
      return ot(!nxt->is_end && !less(k, nxt->key) ?
		std::optional<V>(nxt->value) :
		std::optional<V>());
    else return ot();
  }

  std::optional<V> find_(const K& k) {
    auto [cur, nxt] = find_location(root, k);
    if (!nxt->is_end && !less(k, nxt->key)) return nxt->value;
    else return {};
  }

  std::optional<V> find(const K& k) {
    return verlib::with_epoch([&] {return find_(k);});
  }

  template<typename AddF>
  void range_(AddF& add, const K& start, const K& end) {
    node* nxt = (root->next).load();
    while (true) {
      node* nxt_nxt = (nxt->next).load(); // prefetch
      if (nxt->is_end || !less(nxt->key, start)) break;
      nxt = nxt_nxt;
#ifdef LazyStamp
      if (verlib::aborted) return;
#endif
    }
    while (!nxt->is_end && !less(end, nxt->key)) {
      add(nxt->key, nxt->value);
      nxt = nxt->next.load();
#ifdef LazyStamp
      if (verlib::aborted) return;
#endif
    }
  }

  ordered_map() : root(flck::New<node>(flck::New<node>(nullptr,true),false)) {}
  ordered_map(size_t n) : root(flck::New<node>(flck::New<node>(nullptr,true),false)) {}

  void print() {
    node* ptr = (root->next).load();
    while (!ptr->is_end) {
      std::cout << ptr->key << ", ";
      ptr = (ptr->next).load();
    }
    std::cout << std::endl;
  }

  void retire_recursive(node* p) {
    if (!p->is_end) retire_recursive(p->next.load());
    flck::Retire<node>(p);
  }

  ~ordered_map() {retire_recursive(root);}
  
  long check() {
    node* ptr = (root->next).load();
    if (ptr->is_end) return 0;
    K k = ptr->key;
    ptr = (ptr->next).load();
    long i = 1;
    while (!ptr->is_end) {
      i++;
      if (!less(k, ptr->key)) {
        std::cout << "bad key: " << k << ", " << ptr->key << std::endl;
        abort();
      }
      k = ptr->key;
      ptr = (ptr->next).load();
    }
    return i;
  }

  static void clear() { flck::clear_pool<node>();}
  //static void reserve(size_t n) { node_pool.reserve(n);}
  static void shuffle(size_t n) { } //node_pool.shuffle(n);}
  static void stats() { flck::pool_stats<node>();}

};
