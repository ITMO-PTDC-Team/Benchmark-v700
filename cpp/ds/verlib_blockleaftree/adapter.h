#ifndef VERLIB_BLOCKLEAF_TREE_ADAPTER_H
#define VERLIB_BLOCKLEAF_TREE_ADAPTER_H

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
               const K& KEY_MIN,
               const K& KEY_MAX,
               const V& VALUE_RESERVED,
               Random64 * const unused1)
    : tree(new ordered_map<K, V>())
    , NO_VALUE(VALUE_RESERVED)
    {}

    ~ds_adapter() {
        delete tree;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {
        // Not needed for this implementation
    }
    
    void deinitThread(const int tid) {
        // Not needed for this implementation
    }

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
            return find(tid, key);
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

    int rangeQuery(const int tid, const K& lo, const K& hi, K * const resultKeys, V * const resultValues) {
        setbench_error("RQ functionality not implemented for this data structure");
    }

    void printSummary() {
        std::cout << "Verlib blockleaf tree summary" << std::endl;
        tree->print();
    }
    
    bool validateStructure() {
        return tree->check() >= 0;
    }

    void printObjectSizes() {
        tree->stats();
    }

    void debugGCSingleThreaded() {
        // Not applicable for this implementation
    }

#ifdef USE_TREE_STATS
    class NodeHandler {
    public:
        typedef typename ordered_map<K,V>::node* NodePtrType;

        NodeHandler(const K& _minKey, const K& _maxKey) {}

        class ChildIterator {
        private:
            NodePtrType left, right;
            bool visitedLeft;
        public:
            ChildIterator(NodePtrType node) 
                : left(node ? node->left.load() : nullptr), 
                  right(node ? node->right.load() : nullptr),
                  visitedLeft(false) {}
            
            bool hasNext() {
                return (!visitedLeft && left) || (visitedLeft && right);
            }
            
            NodePtrType next() {
                if (!visitedLeft && left) {
                    visitedLeft = true;
                    return left;
                }
                if (visitedLeft && right) {
                    return right;
                }
                return nullptr;
            }
        };

        bool isLeaf(NodePtrType node) {
            return node && node->is_leaf;
        }
        
        size_t getNumChildren(NodePtrType node) {
            if (!node || node->is_leaf) return 0;
            return (node->left.load() ? 1 : 0) + (node->right.load() ? 1 : 0);
        }
        
        size_t getNumKeys(NodePtrType node) {
            if (!node || !node->is_leaf) return 0;
            return static_cast<ordered_map<K,V>::leaf*>(node)->size;
        }
        
        size_t getSumOfKeys(NodePtrType node) {
            if (!node || !node->is_leaf) return 0;
            auto leaf = static_cast<ordered_map<K,V>::leaf*>(node);
            size_t sum = 0;
            for (int i = 0; i < leaf->size; i++) {
                sum += leaf->keyvals[i].key;
            }
            return sum;
        }
        
        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };
    
    TreeStats<NodeHandler> * createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), tree->root, true);
    }
#endif
};

#endif // VERLIB_BLOCKLEAF_TREE_ADAPTER_H