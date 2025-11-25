//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "workloads/stop_condition/stop_condition.h"
#include "globals_t.h"

namespace microbench::workload {

using K = int64_t;

#if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)

class ThreadLoop {
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

    ThreadLoop(globals_t* g, size_t thread_id, StopCondition* stop_condition, size_t rq_range)
        : g(g),
          threadId(thread_id),
          stopCondition(stop_condition),
          RQ_RANGE(rq_range) {
    }

    template<typename K>
    K* execute_push(const K& key);

    template<typename K>
    K* execute_pop();

    template <typename K>
    K* execute_get(const K& key);

    template <typename K>
    bool execute_contains(const K& key);

    template<typename K>
    K* execute_push(const K& key);

    template<typename K>
    K* execute_pop();

    virtual void run();

    virtual void step() = 0;
};

#endif

}  // namespace microbench::workload
