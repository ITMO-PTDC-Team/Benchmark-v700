//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_THREAD_LOOP_IMPL_H
#define SETBENCH_THREAD_LOOP_IMPL_H

#include "adapter.h"
#include "globals_t_impl.h"
#include "globals_extern.h"

#define THREAD_MEASURED_PRE \
    tid = this->threadId; \
    binding_bindThread(tid); \
    garbage = 0; \
    rqResultKeys = new K[this->RQ_RANGE+MAX_KEYS_PER_NODE]; \
    rqResultValues = new VALUE_TYPE[this->RQ_RANGE+MAX_KEYS_PER_NODE]; \
    NO_VALUE = this->g->dsAdapter->getNoValue();                  \
    __RLU_INIT_THREAD; \
    __RCU_INIT_THREAD; \
    this->g->dsAdapter->initThread(threadId); \
    papi_create_eventset(tid); \
    __sync_fetch_and_add(&this->g->running, 1); \
    __sync_synchronize(); \
    while (!this->g->start) { SOFTWARE_BARRIER; TRACE COUTATOMICTID("waiting to start"<<std::endl); } \
    GSTATS_SET(tid, time_thread_start, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - this->g->startTime).count()); \
    papi_start_counters(tid); \
    int cnt = 0; \
    rq_cnt = 0; \
    DURATION_START(tid);

#define THREAD_MEASURED_POST \
    __sync_fetch_and_add(&this->g->running, -1); \
    DURATION_END(tid, duration_all_ops); \
    GSTATS_SET(tid, time_thread_terminate, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - this->g->startTime).count()); \
    SOFTWARE_BARRIER; \
    papi_stop_counters(tid); \
    SOFTWARE_BARRIER; \
    while (this->g->running) { SOFTWARE_BARRIER; } \
    this->g->dsAdapter->deinitThread(tid); \
    __RCU_DEINIT_THREAD; \
    __RLU_DEINIT_THREAD; \
    delete[] rqResultKeys; \
    delete[] rqResultValues; \
    this->g->garbage += garbage;


template<typename K>
K *ThreadLoop::executeInsert(K &key) {
    TRACE COUTATOMICTID("### calling INSERT " << key << std::endl);


    VALUE_TYPE value = g->dsAdapter->insertIfAbsent(threadId, key, KEY_TO_VALUE(key));
//    K *value = (K *) g->dsAdapter->insertIfAbsent(threadId, key, KEY_TO_VALUE(key));

    if (value == g->dsAdapter->getNoValue()) {
        TRACE COUTATOMICTID("### completed INSERT modification for " << key << std::endl);
        GSTATS_ADD(threadId, key_checksum, key);
//             GSTATS_ADD(tid, size_checksum, 1);
        GSTATS_ADD(threadId, num_successful_inserts, 1);
    } else {
        TRACE COUTATOMICTID("### completed READ-ONLY" << std::endl);
        GSTATS_ADD(threadId, num_fail_inserts, 1);
    }
    GSTATS_ADD(threadId, num_inserts, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return (K *) value;
}

template<typename K>
K *ThreadLoop::executeRemove(const K &key) {
    TRACE COUTATOMICTID("### calling ERASE " << key << std::endl);
//    K *value = (K *) g->dsAdapter->erase(this->threadId, key);
    VALUE_TYPE value = g->dsAdapter->erase(this->threadId, key);

    if (value != this->g->dsAdapter->getNoValue()) {
        TRACE COUTATOMICTID("### completed ERASE modification for " << key << std::endl);
        GSTATS_ADD(threadId, key_checksum, -key);
//             GSTATS_ADD(tid, size_checksum, -1);
        GSTATS_ADD(threadId, num_successful_removes, 1);
    } else {
        TRACE COUTATOMICTID("### completed READ-ONLY" << std::endl);
        GSTATS_ADD(threadId, num_fail_removes, 1);
    }
    GSTATS_ADD(threadId, num_removes, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return (K *) value;
}

template<typename K>
K *ThreadLoop::executeGet(const K &key) {
//    K *value = (K *) this->g->dsAdapter->find(this->threadId, key);
    VALUE_TYPE value = this->g->dsAdapter->find(this->threadId, key);

    if (value != this->g->dsAdapter->getNoValue()) {
        garbage += key; // prevent optimizing out
        GSTATS_ADD(threadId, num_successful_searches, 1);
    } else {
        GSTATS_ADD(threadId, num_fail_searches, 1);
    }
    GSTATS_ADD(threadId, num_searches, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return (K *) value;
}

template<typename K>
bool ThreadLoop::executeContains(const K &key) {
    bool value = this->g->dsAdapter->contains(this->threadId, key);

    if (value) {
        garbage += key; // prevent optimizing out
        GSTATS_ADD(threadId, num_successful_searches, 1);
    } else {
        GSTATS_ADD(threadId, num_fail_searches, 1);
    }
    GSTATS_ADD(threadId, num_searches, 1);
    GSTATS_ADD(threadId, num_operations, 1);

    return value;
}

/**
 * the result is in the arrays rqResultKeys and rqResultValues
 */
template<typename K>
void ThreadLoop::executeRangeQuery(const K &leftKey, const K &rightKey) {
    ++rq_cnt;
    size_t rqcnt;
    if ((rqcnt = this->g->dsAdapter->rangeQuery(this->threadId, leftKey, rightKey,
                                                rqResultKeys, (VALUE_TYPE*) rqResultValues))) {
        garbage += rqResultKeys[0] +
                   rqResultKeys[rqcnt - 1]; // prevent rqResultValues and count from being optimized out
    }
    GSTATS_ADD(threadId, num_rq, 1);
    GSTATS_ADD(threadId, num_operations, 1);
}

void ThreadLoop::run() {
    THREAD_MEASURED_PRE
    while (!stopCondition->isStopped(threadId)) {
        ++cnt;
        VERBOSE if (cnt && ((cnt % 1000000) == 0)) COUTATOMICTID("op# " << cnt << std::endl);
        step();
    }
    THREAD_MEASURED_POST
}


#endif //SETBENCH_THREAD_LOOP_IMPL_H
