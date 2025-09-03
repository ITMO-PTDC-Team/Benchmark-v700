#ifndef VERLIB_OLD_ARTTREE_ADAPTER_H
#define VERLIB_OLD_ARTTREE_ADAPTER_H

#include <iostream>
#include "errors.h"
#include "set.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif

int MAX_RANGE_QUERY_SIZE = 1024;

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    Set<K, V>* tree;
    typename Set<K, V>::node* root;
    const V NO_VALUE;

public:
    ds_adapter(const int NUM_THREADS,
               const K& unused1,
               const K& unused2,
               const V& unused3,
               Random64* const unused4)
    : tree(new Set<K, V>())
    , root(tree->empty())
    , NO_VALUE(unused3)
    {}

    ~ds_adapter() {
        tree->retire(root);
        delete tree;
    }

    V getNoValue() {
        return NO_VALUE;
    }

    void initThread(const int tid) {}
    void deinitThread(const int tid) {}

    bool contains(const int tid, const K& key) {
        return tree->find(root, key).has_value();
    }

    V insert(const int tid, const K& key, const V& val) {
        if (tree->insert(root, key, val)) {
            return NO_VALUE;
        }
        return NO_VALUE;
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        auto result = tree->find(root, key);
        if (result.has_value()) {
            return result.value(); 
        }
        if (tree->insert(root, key, val)) {
            return NO_VALUE; 
        } else {
            return val;
        }
        return NO_VALUE; 
    }

    V erase(const int tid, const K& key) {
        auto result = tree->find(root, key);
        if (result.has_value()) {
            if (tree->remove(root, key)) {
                return result.value();
            }
        }
        return NO_VALUE;
    }

    V find(const int tid, const K& key) {
        auto result = tree->find(root, key);
        return result.has_value() ? result.value() : NO_VALUE;
    }

    int rangeQuery(const int tid, const K& lo, const K& hi, K* const resultKeys, V* const resultValues) {
        return verlib::with_snapshot([&] {
            int count = 0;
            auto add = [&](const K& key, const V& val) {
                if (count < MAX_RANGE_QUERY_SIZE) {
                    resultKeys[count] = key;
                    resultValues[count] = val;
                    count++;
                }
            };
            tree->range_(root, add, lo, hi);
            return count;
        });
    }

    void printSummary() {
        // std::cout << "Verlib old ART-Tree summary" << std::endl;
        // tree->print(root);
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
            return node && node->nt == Set<K,V>::Leaf;
        }
        
        size_t getNumChildren(NodePtrType node) {
            if (!node) return 0;
            switch(node->nt) {
                case Set<K,V>::Full: return 256;
                case Set<K,V>::Indirect: return static_cast<typename Set<K,V>::indirect_node*>(node)->num_used.load();
                case Set<K,V>::Sparse: return static_cast<typename Set<K,V>::sparse_node*>(node)->num_used;
                default: return 0;
            }
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
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), root, true);
    }
#endif
};

#endif // VERLIB_OLD_ARTTREE_ADAPTER_H