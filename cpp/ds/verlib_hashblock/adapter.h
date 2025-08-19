#ifndef VERLIB_HASHBLOCK_ADAPTER_H
#define VERLIB_HASHBLOCK_ADAPTER_H

#include <iostream>
#include "errors.h"
#include "set.h"
#ifdef USE_TREE_STATS
include "tree_stats.h"

#endif

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    unordered_map<K, V>* map;
    const V NO_VALUE;

public:
    ds_adapter(const int NUM_THREADS,
    const K& unused1,
    const K& unused2,
    const V& unused3,
    Random64* const unused4)
    : map(new unordered_map<K, V>(NUM_THREADS * 2))
    , NO_VALUE(unused3)
    {}

    ~ds_adapter() {
        delete map;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {}

    void deinitThread(const int tid) {}

    bool contains(const int tid, const K& key) {
        return map->find(key).has_value();
    }

    V insert(const int tid, const K& key, const V& val) {
        if (map->insert(key, val)) {
            return NO_VALUE;
        }
        return NO_VALUE;
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        auto result = map->find(key);
        if (result.has_value()) {
            return result.value();
        }
        if (map->insert(key, val)) {
            return NO_VALUE;
        }
        return NO_VALUE;
    }

    V erase(const int tid, const K& key) {
        auto result = map->find(key);
        if (result.has_value()) {
            if (map->remove(key)) {
                return result.value();
            }
        }
        return NO_VALUE;
    }

    V find(const int tid, const K& key) {
        auto result = map->find(key);
        return result.has_value() ? result.value() : NO_VALUE;
    }

    int rangeQuery(const int tid, const K& lo, const K& hi, K* const resultKeys, V* const resultValues) {
        setbench_error("RQ functionality not implemented for this data structure");
    }

    void printSummary() {
        std::cout << "Verlib hash block summary" << std::endl;
        map->print();
    }

    bool validateStructure() {
        return true;
    }

    void printObjectSizes() {
        map->stats();
    }

void debugGCSingleThreaded() {}

#ifdef USE_TREE_STATS
    class NodeHandler {
    public:
    typedef typename unordered_map<K,V>::node* NodePtrType;
    text

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
            return node ? node->cnt : 0;
        }
        
        size_t getSumOfKeys(NodePtrType node) {
            size_t sum = 0;
            if (node) {
                if (node->cnt <= 31) {
                    for (int i = 0; i < node->cnt; i++) {
                        sum += node->entries[i].key;
                    }
                } else {
                    auto big_node = static_cast<typename unordered_map<K,V>::BigNode*>(node);
                    for (int i = 0; i < node->cnt; i++) {
                        sum += big_node->entries[i].key;
                    }
                }
            }
            return sum;
        }
        
        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };

    TreeStats<NodeHandler>* createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), nullptr, true);
}

#endif
};

#endif // VERLIB_HASHBLOCK_ADAPTER_H