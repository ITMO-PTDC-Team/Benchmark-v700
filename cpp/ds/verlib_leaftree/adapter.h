#ifndef VERLIB_LEAFTREE_ADAPTER_H
#define VERLIB_LEAFTREE_ADAPTER_H

#include <iostream>
#include "errors.h"
#include "set.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    ordered_map<K, V>* tree;
    const V NO_VALUE;

public:
    ds_adapter(const int NUM_THREADS,
               const K& unused1,
               const K& unused2,
               const V& unused3,
               Random64* const unused4)
    : tree(new ordered_map<K, V>())
    , NO_VALUE(unused3)
    {}

    ~ds_adapter() {
        delete tree;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {}
    
    void deinitThread(const int tid) {}

    bool contains(const int tid, const K& key) {
        return tree->find(key).has_value();
    }

    V insert(const int tid, const K& key, const V& val) {
        setbench_error("Plain insert functionality not implemented for this data structure");
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        auto result = tree->find(key);
        if (result.has_value()) {
            return result.value(); 
        }
        if (tree->insert(key, val)) {
            return NO_VALUE; 
        } else {
            return val;
        }
        return NO_VALUE; 
    }

    V erase(const int tid, const K& key) {
        auto result = tree->find(key);
        if (result.has_value()) {
            if (tree->remove(key)) {
                return result.value();
            }
        }
        return NO_VALUE;
    }

    V find(const int tid, const K& key) {
        auto result = tree->find(key);
        return result.has_value() ? result.value() : NO_VALUE;
    }

    int rangeQuery(const int tid, const K& lo, const K& hi, K* const resultKeys, V* const resultValues) {
        setbench_error("RQ functionality not implemented for this data structure");
    }

    void printSummary() {
        // std::cout << "Verlib leaftree summary" << std::endl;
        // tree->print();
    }

    bool validateStructure() {
        return true;
    }

    void printObjectSizes() {
        tree->stats();
    }

#ifdef USE_TREE_STATS
    class NodeHandler {
    public:
        typedef typename ordered_map<K,V>::node* NodePtrType;

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
            return node && node->is_leaf;
        }
        
        size_t getNumChildren(NodePtrType node) {
            if (!node || node->is_leaf) return 0;
            auto internal = static_cast<typename ordered_map<K,V>::internal*>(node);
            return (internal->left.load() ? 1 : 0) + (internal->right.load() ? 1 : 0);
        }
        
        size_t getNumKeys(NodePtrType node) {
            return isLeaf(node) ? 1 : 0;
        }
        
        size_t getSumOfKeys(NodePtrType node) {
            return node && !node->is_sentinal ? node->key : 0;
        }
        
        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };
    
    TreeStats<NodeHandler>* createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), tree->root, true);
    }
#endif
};

#endif // VERLIB_LEAFTREE_ADAPTER_H