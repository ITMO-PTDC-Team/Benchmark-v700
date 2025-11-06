//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_GLOBALS_T_IMPL_H
#define SETBENCH_GLOBALS_T_IMPL_H

typedef long long test_type;

//#ifdef REDIS
//    #define VALUE_TYPE test_type
//    #define KEY_TO_VALUE(key) key
//#else
//    #define VALUE_TYPE void *
//    #define KEY_TO_VALUE(key) &key /* note: hack to turn a key into a pointer */
//#endif
//
//#define DS_ADAPTER_T ds_adapter<test_type, VALUE_TYPE, RECLAIM<>, ALLOC<>, POOL<> >

//#ifndef INSERT_FUNC
//#define INSERT_FUNC insertIfAbsent
//#endif

#include <chrono>
#include "globals_extern.h"
#include "workloads/bench_parameters.h"
#include "adapter.h"
#include "globals_t.h"

struct globals_t {
    PAD;
    // const
//    void *const NO_VALUE;
    VALUE_TYPE const NO_VALUE;
    const test_type KEY_MIN; // must be smaller than any key that can be inserted/deleted
    const test_type KEY_MAX; // must be less than std::max(), because the snap collector needs a reserved key larger than this! (and larger than any key that can be inserted/deleted)
    const long long PREFILL_INTERVAL_MILLIS;
    PAD;
    // write once
    long long elapsedMillis;
    long long curKeySum = 0;
    long long curSize = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> programExecutionStartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    PAD;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    long long startClockTicks;
    PAD;
    long elapsedMillisNapping;
    PAD;
    volatile test_type garbage; // used to prevent optimizing out some code
    PAD;
    DS_ADAPTER_T *dsAdapter; // the data structure
    PAD;
    BenchParameters * benchParameters;
    PAD;
    Random64 rngs[MAX_THREADS_POW2]; // create per-thread random number generators (padded to avoid false sharing)
//    PAD; // not needed because of padding at the end of rngs
    volatile bool start;
    PAD;
    volatile bool done;
    PAD;
    volatile int running; // number of threads that are running
    PAD;
    volatile bool debug_print;
    PAD;

    globals_t(BenchParameters * _benchParameters)
            : NO_VALUE(NULL), KEY_MIN(0) /*std::numeric_limits<test_type>::min()+1)*/
            , KEY_MAX(_benchParameters->range + 1), PREFILL_INTERVAL_MILLIS(200),
              benchParameters(_benchParameters) {
        debug_print = 0;
        srand(time(0));
        for (int i = 0; i < MAX_THREADS_POW2; ++i) {
            rngs[i].setSeed(rand());
        }

        start = false;
        done = false;
        running = 0;
        dsAdapter = NULL;
        garbage = 0;
        curKeySum = 0;
        curSize = 0;
    }

    void enable_debug_print() {
        debug_print = 1;
    }

    void disable_debug_print() {
        debug_print = 0;
    }

    ~globals_t() {
        delete benchParameters;
    }
};

#endif //SETBENCH_GLOBALS_T_IMPL_H
