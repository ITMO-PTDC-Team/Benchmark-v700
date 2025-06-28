// MIT license (https://opensource.org/license/mit/)
// Initial Author: Guy Blelloch

// A lock free concurrent unordered_map using a hash table
// Supports: fast atomic insert, upsert, remove, and  find
// along with a non-atomic, and slow, size
// Each bucket points to a structure (Node) containing an array of entries
// Nodes come in varying sizes and on update the node is copied.
// Allows arbitrary growing, but only efficient if not much larger
// than the original given size (i.e. number of buckets is fixes, but
// number of entries per bucket can grow).

#include <parlay/primitives.h>
#include <verlib/verlib.h>

template <typename K,
	  typename V,
	  class Hash = std::hash<K>,
	  class KeyEqual = std::equal_to<K>>
struct unordered_map {
private:
  struct KV {K key; V value;};

  template <typename Range>
  static int find_in_range(const Range& entries, long cnt, const K& k) {
    for (int i=0; i < cnt; i++)
      if (KeyEqual{}(entries[i].key, k)) return i;
    return -1;
  }

  // The following three functions copy a range and
  // insert/update/remove the specified key.  No ordering is assumed
  // within the range.  Insert assumes k does not appear, while
  // update/remove assume it does appear.
  template <typename Range, typename RangeIn>
  static void copy_and_insert(Range& out, const RangeIn& entries, long cnt, const K& k, const V& v) {
    for (int i=0; i < cnt; i++) out[i] = entries[i];
    out[cnt] = KV{k,v};
  }

  template <typename Range, typename RangeIn, typename F>
  static void copy_and_update(Range& out, const RangeIn& entries, long cnt, const K& k, const F& f) {
    int i = 0;
    while (!KeyEqual{}(k, entries[i].key) && i < cnt) {
      assert(i < cnt);
      out[i] = entries[i];
      i++;
    }
    out[i].key = entries[i].key;
    out[i].value = f(entries[i].value);
    i++;
    while (i < cnt) {
      out[i] = entries[i];
      i++;
    }
  }

  template <typename Range, typename RangeIn>
  static void copy_and_remove(Range& out, const RangeIn& entries, long cnt, const K& k) {
    int i = 0;
    while (!KeyEqual{}(k, entries[i].key)) {
      assert(i < cnt);
      out[i] = entries[i];
      i++;
    }
    while (i < cnt-1) {
      out[i] = entries[i+1];
      i++;
    }
  }

  // Each bucket points to a Node of some Size, or to a BigNode (defined below)
  // A node contains an array of up to Size entries (actual # of entries given by cnt)
  // Sizes are 1, 3, 7, 31
  template <int Size>
  struct Node : verlib::versioned {
    using node = Node<0>;
    int cnt;
    KV entries[Size];

    // return index of key in entries, or -1 if not found
    int find_index(const K& k) {
      if (cnt <= 31) return find_in_range(entries, cnt, k);
      else return find_in_range(((BigNode*) this)->entries, cnt, k);
    }

    // return optional value found in entries given a key
    std::optional<V> find(const K& k) {
      if (cnt <= 31) { // regular node
	if (KeyEqual{}(entries[0].key, k)) // shortcut for common case
	   return entries[0].value;
	int i = find_in_range(entries+1, cnt-1, k);
	if (i == -1) return {};
	else return entries[i].value;
      } else { // big node
	int i = find_in_range(((BigNode*) this)->entries, cnt, k);
	if (i == -1) return {};
	else return ((BigNode*) this)->entries[i].value;
      }
    }

    // copy and insert
    Node(node* old, const K& k, const V& v) {
      cnt = (old == nullptr) ? 1 : old->cnt + 1;
      copy_and_insert(entries, old->entries, cnt-1, k, v);
    }

    // copy and update
    template <typename F>
    Node(node* old, const K& k, const F& f) : cnt(old->cnt) {
      assert(old != nullptr);
      copy_and_update(entries, old->entries, cnt, k, f);
    }

    // copy and remove
    Node(node* old, const K& k) : cnt(old->cnt - 1) {
      if (cnt == 31) copy_and_remove(entries, ((BigNode*) old)->entries, cnt+1, k);
      else copy_and_remove(entries, old->entries, cnt+1, k);
    }
  };
  using node = Node<0>;

  // If a node overflows (cnt > 31), then it becomes a big node and its content
  // is stored indirectly in a parlay sequence.
  struct BigNode : verlib::versioned {
    using entries_type = std::vector<KV>;
    int cnt;
    entries_type entries;

    // copy and insert
    BigNode(node* old, const K& k, const V& v) : cnt(old->cnt + 1) {
      entries = entries_type(cnt);
      if (old->cnt == 31) copy_and_insert(entries, old->entries, old->cnt, k, v);
      else copy_and_insert(entries, ((BigNode*) old)->entries, old->cnt, k, v);
    }

    // copy and update
    template <typename F>
    BigNode(node* old, const K& k, const F& f) : cnt(old->cnt) {
      entries = entries_type(cnt);
      copy_and_update(entries, ((BigNode*) old)->entries, cnt, k, f);  }

    // copy and remove
    BigNode(node* old, const K& k) : cnt(old->cnt - 1) {
      entries = entries_type(cnt);
      copy_and_remove(entries, ((BigNode*) old)->entries, cnt+1, k); }
  };

  // structure for each bucket
  struct bucket
 #ifndef UseCAS
     : verlib::lock
 #endif
  {
    node* load() {return ptr.load();}
    verlib::versioned_ptr<node> ptr;
    bucket() : ptr(nullptr) {}
  };


  struct Table {
    std::vector<bucket> table;
    size_t size;
    bucket* get_bucket(const K& k) {
      size_t idx = Hash{}(k)  & (size-1u);
      return &table[idx];
    }
    Table(size_t n) {
      int bits = parlay::log2_up(n);
      size = 1ul << bits;
      table = std::vector<bucket>(size);
    }
  };

  Table hash_table;

  using Node1 = Node<1>;
  using Node3 = Node<3>;
  using Node7 = Node<7>;
  using Node31 = Node<31>;
  static verlib::memory_pool<Node1> node_pool_1;
  static verlib::memory_pool<Node3> node_pool_3;
  static verlib::memory_pool<Node7> node_pool_7;
  static verlib::memory_pool<Node31> node_pool_31;
  static verlib::memory_pool<BigNode> big_node_pool;

  static node* insert_to_node(node* old, const K& k, const V& v) {
    if (old == nullptr) return (node*) node_pool_1.new_obj(old, k, v);
    if (old->cnt < 3) return (node*) node_pool_3.new_obj(old, k, v);
    else if (old->cnt < 7) return (node*) node_pool_7.new_obj(old, k, v);
    else if (old->cnt < 31) return (node*) node_pool_31.new_obj(old, k, v);
    else return (node*) big_node_pool.new_obj(old, k, v);
  }

  template <typename F>
  static node* update_node(node* old, const K& k, const F& f) {
    if (old->cnt == 1) return (node*) node_pool_1.new_obj(old, k, f);
    if (old->cnt <= 3) return (node*) node_pool_3.new_obj(old, k, f);
    else if (old->cnt <= 7) return (node*) node_pool_7.new_obj(old, k, f);
    else if (old->cnt <= 31) return (node*) node_pool_31.new_obj(old, k, f);
    else return (node*) big_node_pool.new_obj(old, k, f);
  }

  static node* remove_from_node(node* old, const K& k) {
    if (old->cnt == 1) return (node*) nullptr;
    if (old->cnt == 2) return (node*) node_pool_1.new_obj(old, k);
    else if (old->cnt <= 4) return (node*) node_pool_3.new_obj(old, k);
    else if (old->cnt <= 8) return (node*) node_pool_7.new_obj(old, k);
    else if (old->cnt <= 32) return (node*) node_pool_31.new_obj(old, k);
    else return (node*) big_node_pool.new_obj(old, k);
  }

  static void retire_node(node* old) {
    if (old == nullptr);
    else if (old->cnt == 1) node_pool_1.retire((Node1*) old);
    else if (old->cnt <= 3) node_pool_3.retire((Node3*) old);
    else if (old->cnt <= 7) node_pool_7.retire((Node7*) old);
    else if (old->cnt <= 31) node_pool_31.retire((Node31*) old);
    else big_node_pool.retire((BigNode*) old);
  }

  static void destruct_node(node* old) {
    if (old == nullptr);
    else if (old->cnt == 1) node_pool_1.destruct((Node1*) old);
    else if (old->cnt <= 3) node_pool_3.destruct((Node3*) old);
    else if (old->cnt <= 7) node_pool_7.destruct((Node7*) old);
    else if (old->cnt <= 31) node_pool_31.destruct((Node31*) old);
    else big_node_pool.destruct((BigNode*) old);
  }

  // try to install a new node in bucket s
  static std::optional<bool> try_update(bucket* s, node* old_node, node* new_node, bool ret_val) {
#ifdef UseCAS
    if (s->load() == old_node &&
	s->ptr.cas(old_node, new_node)) 
#else  // use try_lock
    if (s->try_lock([=] {
	    if (s->load() != old_node) return false;
	    s->ptr = new_node;
	    return true;})) 
#endif
    {
      retire_node(old_node);
      return ret_val;
    } 
    destruct_node(new_node);
    return {};
  }

  static std::optional<bool> try_insert_at(bucket* s, const K& k, const V& v) {
    node* old_node = s->load();
    if (old_node != nullptr && old_node->find_index(k) != -1) {
#ifndef UseCAS
      if (s->read_lock([&] {return s->load() == old_node;})) return false;
      else return {};
#else
      return false;
#endif
    }
    return try_update(s, old_node, insert_to_node(old_node, k, v), true);
  }

  template <typename F>
  static std::optional<bool> try_upsert_at(bucket* s, const K& k, F& f) {
    node* old_node = s->load();
    bool found = old_node != nullptr && old_node->find_index(k) != -1;
    if (!found)
      return try_update(s, old_node, insert_to_node(old_node, k, f(std::optional<V>())), true);
    else
#ifdef UseCAS
    return try_update(s, old_node, update_node(old_node, k, f), false);
#else  // use try_lock
    if (s->try_lock([=] {
        if (s->load() != old_node) return false;
	s->ptr = update_node(old_node, k, f); // f applied within lock
	return true;})) {
      retire_node(old_node);
      return false;
    } else return {};
#endif
  }

  static std::optional<bool> try_remove_at(bucket* s, const K& k) {
    node* old_node = s->load();
    if (old_node == nullptr || old_node->find_index(k) == -1) {
#ifndef UseCAS
      if (s->read_lock([&] {return s->load() == old_node;})) return false;
      else return {};
#else
      return false;
#endif
    }
    return try_update(s, old_node, remove_from_node(old_node, k), true);
  }

  // find a key at the given bucket
  static std::optional<V> find_at(node* x, const K& k) {
    if (x == nullptr) return std::nullopt;
    return x->find(k);
  }

public:
  unordered_map(size_t n) : hash_table(Table(n)) {}
  ~unordered_map() {
    auto& table = hash_table.table;
    parlay::parallel_for (0, table.size(), [&] (size_t i) {
      retire_node(table[i].load());});
  }
  
  std::optional<V> find(const K& k) {
    bucket* s = hash_table.get_bucket(k);
    __builtin_prefetch (s);
    return verlib::with_epoch([&] {
		return find_at(s->load(), k);});
  }

  std::optional<V> find_(const K& k) {
    bucket* s = hash_table.get_bucket(k);
    return find_at(s->load(), k);
  }

  std::optional<V> find_locked(const K& k) {
    bucket* s = hash_table.get_bucket(k);
    return find_at(s->read_lock([=] {return s->load();}), k);
  }
      
  bool insert(const K& k, const V& v) {
    bucket* s = hash_table.get_bucket(k);
    __builtin_prefetch (s);
    return verlib::with_epoch([&] {
      return flck::try_loop([&] {return try_insert_at(s, k, v);});});
  }

  bool insert_(const K& k, const V& v) {
    bucket* s = hash_table.get_bucket(k);
    return flck::try_loop([&] {return try_insert_at(s, k, v);});
  }

  template <typename F>
  bool upsert(const K& k, const F& f) {
    bucket* s = hash_table.get_bucket(k);
    __builtin_prefetch (s);
    return verlib::with_epoch([&] {
      return flck::try_loop([&] {return try_upsert_at(s, k, f);});});
  }

  template <typename F>
  bool upsert_(const K& k, const F& f) {
    bucket* s = hash_table.get_bucket(k);
    return flck::try_loop([&] {return try_upsert_at(s, k, f);});
  }

  bool remove(const K& k) {
    bucket* s = hash_table.get_bucket(k);
    __builtin_prefetch (s);
    return verlib::with_epoch([&] {
      return flck::try_loop([&] {return try_remove_at(s, k);});});
  }

  bool remove_(const K& k) {
    bucket* s = hash_table.get_bucket(k);
    return flck::try_loop([&] {return try_remove_at(s, k);});
  }

  long size() {
    auto& table = hash_table.table;
    auto s = parlay::tabulate(table.size(), [&] (size_t i) {
	      node* x = table[i].load();
	      if (x == nullptr) return 0;
	      else return x->cnt;});
    return parlay::reduce(s);
  }

  void print() {
    auto& table = hash_table.table;
    for (size_t i=0; i < table.size(); i++) {
      node* x = table[i].ptr.load();
      if (x != nullptr)
	for (int i = 0; i < x->cnt; i++) {
	  K key = (x->cnt < 32) ? x->entries[i].key : ((BigNode*) x)->entries[i].key;
	  std::cout << key << ", ";
	}
    }
    std::cout << std::endl;
  }

  long check() { return size();}

  static void clear() {
    node_pool_1.clear();
    node_pool_3.clear();
    node_pool_7.clear();
    node_pool_31.clear();
    big_node_pool.clear();
  }
  static void stats() {
    node_pool_1.stats();
    node_pool_3.stats();
    node_pool_7.stats();
    node_pool_31.stats();
    big_node_pool.stats();
  }
  static void reserve(size_t n) {}
  static void shuffle(size_t n) {}

};

template <typename K, typename V, typename H, typename E>
verlib::memory_pool<typename unordered_map<K,V,H,E>::Node1> unordered_map<K,V,H,E>::node_pool_1;
template <typename K, typename V, typename H, typename E>
verlib::memory_pool<typename unordered_map<K,V,H,E>::Node3> unordered_map<K,V,H,E>::node_pool_3;
template <typename K, typename V, typename H, typename E>
verlib::memory_pool<typename unordered_map<K,V,H,E>::Node7> unordered_map<K,V,H,E>::node_pool_7;
template <typename K, typename V, typename H, typename E>
verlib::memory_pool<typename unordered_map<K,V,H,E>::Node31> unordered_map<K,V,H,E>::node_pool_31;
template <typename K, typename V, typename H, typename E>
verlib::memory_pool<typename unordered_map<K,V,H,E>::BigNode> unordered_map<K,V,H,E>::big_node_pool;
