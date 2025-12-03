#pragma once

#include <iostream>
#include <csignal>
#include "random_xoshiro256p.h"
#include <bits/stdc++.h>
using namespace std;

#include "errors.h"
#include "record_manager.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif
#include "stack.h"

#define DATA_STRUCTURE_T mstack

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    const V NO_VALUE;
    DATA_STRUCTURE_T * const ds;

public:
    ds_adapter(const int NUM_THREADS,
               const K& KEY_MIN,
               const K& KEY_MAX,
               const V& VALUE_RESERVED,
               Random64 * const unused2)
            : NO_VALUE(VALUE_RESERVED)
            , ds(new DATA_STRUCTURE_T)
    { }

    ~ds_adapter() {
        delete ds;
    }

    V getNoValue() {
        return 0;
    }

    void initThread(const int tid) {

    }
    void deinitThread(const int tid) {
        
    }

    void setCops(const int tid, int cops) {
        
    }

    void warmupEnd() {
    }

    V insert(const int tid, const K& key, const V& val) {
        setbench_error("insert-replace functionality not implemented for this data structure");
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        setbench_error("insert-replace functionality not implemented for this data structure");
    }

    V erase(const int tid, const K& key) {
        setbench_error("not implemented");
    }

    V find(const int tid, const K& key) {
        return ds->find(tid, key);
    }

    V push(const int tid, const K& key) {
        return ds->push(tid, key);
    }

    V pop(const int tid) {
        return ds->pop(tid);
    }

    long long getPathsLength(const int tid) {
        // TODO: Check this in stack
        return 0;
    }

    bool contains(const int tid, const K& key) {
        // return ds->contains(tid, key);
    }

    int rangeQuery(const int tid, const K& lo, const K& hi, K * const resultKeys, V * const resultValues) {
        setbench_error("not implemented");
    }
    void printSummary() {
//        ds->printDebuggingDetails();
    }
    bool validateStructure() {
        // return ds->validate();
        return true
    }
    int getHeight() {
        // TODO: Check this in stack
        return 0;
    }

    void printObjectSizes() {
        std::cout<<"sizes: node="
                 <<(sizeof(mstack_node))
                 <<std::endl;
    }

    std::vector<pair<int, int> > getPairsKeyHeight() {
        
    }

    std::vector<pair<int, int> > getPairsKeyContains() {

    }
};
