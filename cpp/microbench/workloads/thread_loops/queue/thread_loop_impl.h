#pragma once

#include "adapter.h"
#include "globals_t_impl.h"

namespace microbench::workload {

#define THREAD_MEASURED_PRE                                                        \
    tid = this->threadId;                                                          \
    binding_bindThread(tid);                                                       \
    garbage = 0;                                                                   \
    NO_VALUE = this->g->dsAdapter->getNoValue();                                   \
    __RLU_INIT_THREAD;                                                             \
    __RCU_INIT_THREAD;                                                             \
    this->g->dsAdapter->initThread(threadId);                                      \
    papi_create_eventset(tid);                                                     \
    __sync_fetch_and_add(&this->g->running, 1);                                    \
    __sync_synchronize();                                                          \
    while (!this->g->start) {                                                      \
        SOFTWARE_BARRIER;                                                          \
        TRACE COUTATOMICTID("waiting to start" << std::endl);                      \
    }                                                                              \
    GSTATS_SET(tid, time_thread_start,                                             \
               std::chrono::duration_cast<std::chrono::microseconds>(              \
                   std::chrono::high_resolution_clock::now() - this->g->startTime) \
                   .count());                                                      \
    papi_start_counters(tid);                                                      \
    int cnt = 0;                                                                   \
    DURATION_START(tid);

#define THREAD_MEASURED_POST                                                       \
    __sync_fetch_and_add(&this->g->running, -1);                                   \
    DURATION_END(tid, duration_all_ops);                                           \
    GSTATS_SET(tid, time_thread_terminate,                                         \
               std::chrono::duration_cast<std::chrono::microseconds>(              \
                   std::chrono::high_resolution_clock::now() - this->g->startTime) \
                   .count());                                                      \
    SOFTWARE_BARRIER;                                                              \
    papi_stop_counters(tid);                                                       \
    SOFTWARE_BARRIER;                                                              \
    while (this->g->running) {                                                     \
        SOFTWARE_BARRIER;                                                          \
    }                                                                              \
    this->g->dsAdapter->deinitThread(tid);                                         \
    __RCU_DEINIT_THREAD;                                                           \
    __RLU_DEINIT_THREAD;                                                           \
    this->g->garbage += garbage;


template <typename K>
K* QueueThreadLoop::execute_get(const K& key) {
    //    K *value = (K *) this->g->dsAdapter->find(this->threadId, key);
    K* value = (K*) this->g->dsAdapter->find(this->threadId, key);

    if (value != this->g->dsAdapter->getNoValue()) {
        garbage += key;  // prevent optimizing out
        GSTATS_ADD(threadId, num_successful_searches, 1);
    } else {
        GSTATS_ADD(threadId, num_fail_searches, 1);
    }
    GSTATS_ADD(threadId, num_searches, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return (K*)value;
}

template <typename K>
K* QueueThreadLoop::execute_push(const K& key) {
    TRACE COUTATOMICTID("### calling PUSH " << key << std::endl);

    auto value = g->dsAdapter->push(threadId, key);
    for (int i = 0; i < this->nopCount; i++) {
        __asm__ __volatile__("nop");
    }
    //    K *value = (K *) g->dsAdapter->insertIfAbsent(threadId, key, KEY_TO_VALUE(key));
    garbage += key;  // prevent optimizing out
    GSTATS_ADD(threadId, num_pushes, 1);
    GSTATS_ADD(threadId, num_operations, 1);
    return (K*)value;
}

template <typename K>
K* QueueThreadLoop::execute_pop() {
    TRACE COUTATOMICTID("### calling POP " << std::endl);
    //    K *value = (K *) this->g->dsAdapter->find(this->threadId, key);
    VALUE_TYPE value = this->g->dsAdapter->pop(this->threadId);

    if (value != this->g->dsAdapter->getNoValue()) {
        TRACE COUTATOMICTID("### completed POP modification for " << value << std::endl);
        GSTATS_ADD(threadId, num_successful_pops, 1);
    } else {
        TRACE COUTATOMICTID("### completed READ-ONLY" << std::endl);
        GSTATS_ADD(threadId, num_fail_pops, 1);
    }
    GSTATS_ADD(threadId, num_pops, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return (K*)value;
}

template <typename K>
bool QueueThreadLoop::execute_contains(const K& key) {
    bool value = this->g->dsAdapter->contains(this->threadId, key);

    if (value) {
        garbage += key;  // prevent optimizing out
        GSTATS_ADD(threadId, num_successful_searches, 1);
    } else {
        GSTATS_ADD(threadId, num_fail_searches, 1);
    }
    GSTATS_ADD(threadId, num_searches, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return value;
}

void QueueThreadLoop::run() {
    THREAD_MEASURED_PRE
    while (!stopCondition->is_stopped(threadId)) {
        ++cnt;
        VERBOSE if (cnt && ((cnt % 1000000) == 0)) COUTATOMICTID("op# " << cnt << std::endl);
        step();
    }
    THREAD_MEASURED_POST
}

}

