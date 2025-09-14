#ifndef VERLIB_LIST_ONELOCK_ADAPTER_H
#define VERLIB_LIST_ONELOCK_ADAPTER_H

#include <iostream>
#include "errors.h"
#include "set.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    Set<K, V>* set;
    typename Set<K, V>::node* root;
    const V NO_VALUE;

public:
    ds_adapter(const int NUM_THREADS,
               const K& KEY_MIN,
               const K& KEY_MAX,
               const V& VALUE_RESERVED,
               Random64* const unused1)
    : set(new Set<K, V>())
    , root(set->empty())
    , NO_VALUE(VALUE_RESERVED)
    {}

    ~ds_adapter() {
        set->retire(root);
        delete set;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {}

    void deinitThread(const int tid) {}

    bool contains(const int tid, const K& key) {
        return set->find(root, key).has_value();
    }

    V insert(const int tid, const K& key, const V& val) {
        setbench_error("Plain insert functionality not implemented for this data structure");
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        if (set->insert(root, key, val)) {
            return NO_VALUE; 
        } else {
            return val;
        }
        return NO_VALUE; 
    }

    V erase(const int tid, const K& key) {
        if (set->remove(root, key)) {
            return reinterpret_cast<V>(const_cast<K*>(&key));
        }
        return NO_VALUE;
    }

    V find(const int tid, const K& key) {
        auto result = set->find(root, key);
        return result.has_value() ? result.value() : NO_VALUE;
    }

    int rangeQuery(const int tid, const K& lo, const K& hi, K* const resultKeys, V* const resultValues) {
        setbench_error("RQ functionality not implemented for this data structure");
    }

    void printSummary() {
        // std::cout << "Verlib list onelock summary" << std::endl;
        // set->print(root);
    }

    bool validateStructure() {
        return true;
    }

    void printObjectSizes() { }

#ifdef USE_TREE_STATS
    class NodeHandler {
    public:
        typedef typename Set<K,V>::node* NodePtrType;

        NodeHandler(const K& _minKey, const K& _maxKey) {}

        class ChildIterator {
        public:
            ChildIterator(NodePtrType _node) {}
            bool hasNext() {
                return false;
            }
            NodePtrType next() {
                return nullptr;
            }
        };

        bool isLeaf(NodePtrType node) {
            return true;
        }
        
        size_t getNumChildren(NodePtrType node) {
            return 0;
        }
        
        size_t getNumKeys(NodePtrType node) {
            return node && !node->is_end ? 1 : 0;
        }
        
        size_t getSumOfKeys(NodePtrType node) {
            return node && !node->is_end ? node->key : 0;
        }
        
        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };
    
    TreeStats<NodeHandler>* createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), root, true);
    }
#endif
};

#endif // VERLIB_LIST_ONELOCK_ADAPTER_H