//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "workloads/stop_condition/stop_condition.h"
#include "workloads/thread_loops/thread_loop.h"
#include "globals_t.h"

namespace microbench::workload {

class QueueThreadLoop : public ThreadLoop {
protected:
    K garbage = 0;
    VALUE_TYPE NO_VALUE;
    size_t nopCount;

public:
    size_t threadId;
    globals_t* g;
    StopCondition* stopCondition;

    QueueThreadLoop(globals_t* g, size_t thread_id, StopCondition* stop_condition, size_t nop_count = 0)
        : g(g),
          threadId(thread_id),
          stopCondition(stop_condition),
          nopCount(nop_count) {
    }

    template <typename K>
    K* execute_get(const K& key);

    template <typename K>
    bool execute_contains(const K& key);

    template<typename K>
    K* execute_push(const K& key);

    template<typename K>
    K* execute_pop();
    
    /**
     * the result is in the arrays rqResultKeys and rqResultValues
     */
    template <typename K>
    void execute_range_query(const K& left_key, const K& right_key);

    virtual void run() override;

    virtual void step() override = 0;
};

}  // namespace microbench::workload