//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <memory>
#include "workloads/stop_condition/stop_condition.h"

namespace microbench {
struct globals_t;
}

namespace microbench::workload {

using KeyType = int64_t;
using ValueType = void*;

class ThreadLoop {
protected:
    KeyType garbage = 0;
    std::vector<KeyType> rqResultKeys;
    std::vector<ValueType> rqResultValues;
    ValueType NO_VALUE;
    int rq_cnt;
    size_t RQ_RANGE;

public:
    size_t threadId;
    globals_t* g;
    StopConditionPtr stopCondition;

    ThreadLoop(globals_t* g, size_t thread_id, StopConditionPtr stop_condition, size_t rq_range)
        : g(g),
          threadId(thread_id),
          stopCondition(std::move(stop_condition)),
          RQ_RANGE(rq_range) {
    }

    KeyType* execute_insert(KeyType& key);

    KeyType* execute_remove(const KeyType& key);

    KeyType* execute_get(const KeyType& key);

    bool execute_contains(const KeyType& key);

    /**
     * the result is in the arrays rqResultKeys and rqResultValues
     */
    void execute_range_query(const KeyType& left_key, const KeyType& right_key);

    virtual void run();

    virtual void step() = 0;
};

#ifndef MAIN_BENCH

void ThreadLoop::execute_range_query(const KeyType& left_key, const KeyType& right_key) {
}

bool ThreadLoop::execute_contains(const KeyType& key) {
    return false;
}

KeyType* ThreadLoop::execute_get(const KeyType& key) {
    return nullptr;
}

KeyType* ThreadLoop::execute_remove(const KeyType& key) {
    return nullptr;
}

KeyType* ThreadLoop::execute_insert(KeyType& key) {
    return nullptr;
}

void ThreadLoop::run() {
}

#endif

using ThreadLoopPtr = std::unique_ptr<ThreadLoop>;

}  // namespace microbench::workload
