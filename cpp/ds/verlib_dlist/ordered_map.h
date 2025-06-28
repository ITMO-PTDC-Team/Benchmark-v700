  #include <verlib/verlib.h>

  template <typename K,
            typename V,
            typename Compare = std::less<K>>
  struct ordered_map {
    static constexpr auto less = Compare{};

    struct alignas(32) node : verlib::versioned, flck::lock {
      verlib::versioned_ptr<node> next;
      // needs to be versioned for transactions?
      flck::atomic<node*> prev;
      K key;
      V value;
      bool is_end;
      verlib::atomic_bool removed;
      verlib::lock lck;
      node(K key, V value, node* next, node* prev)
        : key(key), value(value), next(next), prev(prev), is_end(false), removed(false) {};
      node(node* next) // for head and tail
        : next(next), is_end(true), removed(false), prev(nullptr) {};
    };

    node* root;
    
    auto find_location(node* root, const K& k) {
      node* next = (root->next).load();
      while (true) {
        node* next_next = (next->next).load(); // prefetch
        if (next->is_end || !less(next->key, k)) break;
        next = next_next;
      }
      return next;
    }

    std::optional<bool> try_insert(const K& k, const V& v) {
      node* next = find_location(root, k);
      node* prev = (next->prev).load();
      if (!next->is_end && !less(k, next->key)) //already there
        if (prev->lck.read_lock([&] {return ((prev->next.load() == next) &&
              !prev->removed.load());}))
    return false;
        else return {};
      if ((prev->is_end || prev->key < k) && 
    prev->try_lock([=] {
      if (!prev->removed.load() && (prev->next).load() == next) {
        node* new_node = flck::New<node>(k, v, next, prev);
        prev->next = new_node;
        next->prev = new_node;
    return true;
        } else return false;})) return true;
      else return {}; // failed
    }

    bool insert_(const K& k, const V& v) {
      return flck::try_loop([&] {return try_insert(k, v);});}
    
    bool insert(const K& k, const V& v) {
      return verlib::with_epoch([=] {return insert_(k, v);}); }

    std::optional<bool> try_remove(const K& k) {
      node* loc = find_location(root, k);
      node* prev = (loc->prev).load();
      if (loc->is_end || less(k, loc->key)) // not found
        if (prev->lck.read_lock([&] {return ((prev->next.load() == loc) &&
              !prev->removed.load());}))
    return false;
        else return {};
      if (prev->try_lock([=] {
          if (prev->removed.load() || (prev->next).load() != loc)
      return false;
    return loc->try_lock([=] {
          node* next = (loc->next).load();
    loc->removed = true;
    prev->next = next;
    next->prev = prev;
    flck::Retire<node>(loc);
    return true;
          });}))
        return true;
      else return {};
    }

    bool remove_(const K& k) {
      return flck::try_loop([&] {return try_remove(k);});}
    
    bool remove(const K& k) {
      return verlib::with_epoch([=] { return remove_(k);});}

    std::optional<std::optional<V>> try_find(const K& k) {
      using ot = std::optional<std::optional<V>>;
      node* next = find_location(root, k);
      node* prev = next->prev.load();
      if (prev->lck.read_lock([&] {return ((prev->next.load() == next) &&
            !prev->removed.load());}))
        return ot(!next->is_end && !less(k, next->key) ?
      std::optional<V>(next->value) :
      std::optional<V>());
      else return ot();
    }

    std::optional<V> find_locked(const K& k) {
      return flck::try_loop([&] {return try_find(k);});
    }

    std::optional<V> find_(const K& k) {
      node* next = find_location(root, k);
      if (!next->is_end && !less(k, next->key)) return next->value;
      else return {};
    }

    std::optional<V> find(const K& k) {
      return verlib::with_epoch([&] {return find_(k);});
    }

    template<typename AddF>
    void range_(AddF& add, const K& start, const K& end) {
      node* next = (root->next).load();
      while (true) {
        node* next_next = (next->next).load(); // prefetch
        if (next->is_end || !less(next->key, start)) break;
        next = next_next;
  #ifdef LazyStamp
        if (verlib::aborted) return;
  #endif
      }
      while (!next->is_end && !less(end, next->key)) {
        add(next->key, next->value);
        next = next->next.load();
  #ifdef LazyStamp
        if (verlib::aborted) return;
  #endif
      }
    }

    node* init() {
      node* tail = flck::New<node>(nullptr);
      node* head = flck::New<node>(tail);
      tail->prev = head;
      return head;
    }
      
    ordered_map() : root(init()) {}
    ordered_map(size_t n) : root(init()) {}

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
    static void shuffle(size_t n) {} // node_pool.shuffle(n);}
    static void stats() { flck::pool_stats<node>();}

  };

