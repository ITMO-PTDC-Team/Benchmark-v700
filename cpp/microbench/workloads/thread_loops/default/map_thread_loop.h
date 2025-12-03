//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "workloads/stop_condition/stop_condition.h"
#include "workloads/thread_loops/thread_loop.h"
#include "globals_t.h"

namespace microbench::workload {

class MapThreadLoop : public ThreadLoop {
protected:
    K garbage = 0;
    K* rqResultKeys;
    VALUE_TYPE* rqResultValues;
    VALUE_TYPE NO_VALUE;
    int rq_cnt;
    size_t RQ_RANGE;

public:
    size_t threadId;
    globals_t* g;
    StopCondition* stopCondition;

    MapThreadLoop() = default;
    
    MapThreadLoop(globals_t* g, size_t thread_id, StopCondition* stop_condition, size_t rq_range)
        : g(g),
          threadId(thread_id),
          stopCondition(stop_condition),
          RQ_RANGE(rq_range) {
    }

    template <typename K>
    K* execute_insert(K& key);

    template <typename K>
    K* execute_remove(const K& key);

    template <typename K>
    K* execute_get(const K& key);

    template <typename K>
    bool execute_contains(const K& key);
    
    /**
     * the result is in the arrays rqResultKeys and rqResultValues
     */
    template <typename K>
    void execute_range_query(const K& left_key, const K& right_key);

    virtual void run() override;

    virtual void step() override = 0;
};

#ifndef MAIN_BENCH

template <typename K>
void MapThreadLoop::execute_range_query(const K& left_key, const K& right_key) {
}

template <typename K>
bool MapThreadLoop::execute_contains(const K& key) {
    return false;
}

template <typename K>
K* MapThreadLoop::execute_get(const K& key) {
    return nullptr;
}

template <typename K>
K* MapThreadLoop::execute_remove(const K& key) {
    return nullptr;
}

template <typename K>
K* MapThreadLoop::execute_insert(K& key) {
    return nullptr;
}

void MapThreadLoop::run() {
}

#endif

}  // namespace microbench::workload