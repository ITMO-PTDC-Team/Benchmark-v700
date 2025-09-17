//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_THREAD_LOOP_H
#define SETBENCH_THREAD_LOOP_H

#include "workloads/stop_condition/stop_condition.h"
#include "workloads/key_converters/key_converter.h"
#include "globals_t.h"

//#define VALUE_TYPE void *

//typedef long long K;

class ThreadLoop {
protected:
    size_t garbage = 0;
    KEY_TYPE * rqResultKeys;
    VALUE_TYPE * rqResultValues;
    VALUE_TYPE NO_VALUE;
    int rq_cnt;
    size_t RQ_RANGE;

public:
    size_t threadId;
    globals_t *g;
    StopCondition *stopCondition;

    ThreadLoop(globals_t *_g, size_t _threadId, StopCondition *_stopCondition, size_t _RQ_RANGE)
            : g(_g), threadId(_threadId), stopCondition(_stopCondition), RQ_RANGE(_RQ_RANGE) {}

    // template<typename K>
    KEY_TYPE * executeInsert(size_t& key);

    // template<typename K>
    KEY_TYPE * executeRemove(size_t& key);

    // template<typename K>
    KEY_TYPE * executeGet(size_t& key);

    //template<typename K>
    bool executeContains(size_t& key);

    /**
     * the result is in the arrays rqResultKeys and rqResultValues
     */
    //template<typename K>
    void executeRangeQuery(size_t& leftKey, size_t& rightKey);

    virtual void run();

    virtual void step() = 0;
};

#ifndef MAIN_BENCH

template<typename K>
void ThreadLoop::executeRangeQuery(const K &leftKey, const K &rightKey) {

}

template<typename K>
bool ThreadLoop::executeContains(const K &key) {
    return false;
}

template<typename K>
K *ThreadLoop::executeGet(const K &key) {
    return nullptr;
}

template<typename K>
K *ThreadLoop::executeRemove(const K &key) {
    return nullptr;
}

template<typename K>
K *ThreadLoop::executeInsert(K &key) {
    return nullptr;
}

void ThreadLoop::run() {

}

#endif

#endif //SETBENCH_THREAD_LOOP_H
