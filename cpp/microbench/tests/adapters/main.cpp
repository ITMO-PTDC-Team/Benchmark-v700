#include <gtest/gtest.h>

#include <fstream>
#include <thread>
#include <limits>
#include <cstring>
#include <ctime>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>
#include <parallel/algorithm>
#include <omp.h>
#include <perftools.h>
#include <regex>
#include <cstdlib>

#define MAIN_BENCH

#ifdef PRINT_JEMALLOC_STATS
#include <jemalloc/jemalloc.h>
#define DEBUG_PRINT_ARENA_STATS malloc_stats_print(printCallback, NULL, "ag")
void printCallback(void * nothing, const char * data) {
    std::cout<<data;
}
#else
#define DEBUG_PRINT_ARENA_STATS
#endif

/******************************************************************************
 * Configure global statistics tracking & output using GSTATS (common/gstats)
 * Note: it is crucial that this import occurs before any user headers
 * (that might use GSTATS) are included.
 *
 * This is because it define the macro GSTATS_HANDLE_STATS, which is
 * (read: should be) used by all user includes to determine whether to perform
 * any GSTATS_ calls.
 *
 * Thus, including this before all other user headers ENABLES GSTATS in them.
 *****************************************************************************/
#include "define_global_statistics.h"
#include "gstats_global.h" // include the GSTATS code and macros (crucial this happens after GSTATS_HANDLE_STATS is defined)
// Note: any statistics trackers created by headers included below have to be handled separately... we do this below.

#ifdef KEY_DEPTH_TOTAL_STAT
int64_t key_depth_total_sum__;
int64_t key_depth_total_cnt__;
#endif

#ifdef KEY_DEPTH_STAT
int64_t* key_depth_sum__ = nullptr;
int64_t* key_depth_cnt__ = nullptr;
#endif

#ifdef KEY_SEARCH_TOTAL_STAT
int64_t key_search_total_iters_cnt__;
int64_t key_search_total_cnt__;
#endif

// each thread saves its own thread-id (should be used primarily within this file--could be eliminated to improve software engineering)
__thread int tid = 0;

#include "plaf.h"

#include "globals_extern.h"
#include "json/single_include/nlohmann/json.hpp"
#include "random_xoshiro256p.h"
#include "binding.h"
#include "papi_util_impl.h"
#include "rq_provider.h"

#include "adapter.h" /* data structure adapter header (selected according to the "ds/..." subdirectory in the -I include paths */
#include "tree_stats.h"

#ifdef USE_RCU
#include "eer_prcu_impl.h"
#define __RCU_INIT_THREAD urcu::registerThread(tid);
#define __RCU_DEINIT_THREAD urcu::unregisterThread();
#define __RCU_INIT_ALL urcu::init(TOTAL_THREADS);
#define __RCU_DEINIT_ALL urcu::deinit(TOTAL_THREADS);
#else
#define __RCU_INIT_THREAD
#define __RCU_DEINIT_THREAD
#define __RCU_INIT_ALL
#define __RCU_DEINIT_ALL
#endif

#ifdef USE_RLU
#include "rlu.h"
PAD;
__thread rlu_thread_data_t * rlu_self;
PAD;
rlu_thread_data_t * rlu_tdata = NULL;
#define __RLU_INIT_THREAD rlu_self = &rlu_tdata[tid]; RLU_THREAD_INIT(rlu_self);
#define __RLU_DEINIT_THREAD RLU_THREAD_FINISH(rlu_self);
#define __RLU_INIT_ALL rlu_tdata = new rlu_thread_data_t[MAX_THREADS_POW2]; RLU_INIT(RLU_TYPE_FINE_GRAINED, 1);
#define __RLU_DEINIT_ALL RLU_FINISH(); delete[] rlu_tdata;
#else
#define __RLU_INIT_THREAD
#define __RLU_DEINIT_THREAD
#define __RLU_INIT_ALL
#define __RLU_DEINIT_ALL
#endif

#define INIT_ALL \
    __RCU_INIT_ALL; \
    __RLU_INIT_ALL;
#define DEINIT_ALL \
    __RLU_DEINIT_ALL; \
    __RCU_DEINIT_ALL;



/******************************************************************************
 * Define global variables to store the numerical IDs of all GSTATS global
 * statistics trackers that have been defined over all files #included.
 *
 * It is CRUCIAL that this occurs AFTER ALL user #includes (so we catch ALL
 * GSTATS statistics trackers/counters/timers defined by those #includes).
 *
 * This includes the statistics trackers defined in define_global_statistics.h
 * as well any that were setup by a particular data structure / allocator /
 * reclaimer / pool / library that was #included above.
 *
 * This is a manually constructed list that you are free to add to if you
 * create, e.g., your own data structure specific statistics trackers.
 * They will only be included / printed when your data structure is active.
 *
 * If you add something here, you must also add to a few similar code blocks
 * below. Search this file for "GSTATS_" and you'll see where...
 *****************************************************************************/
GSTATS_DECLARE_ALL_STAT_IDS;
#ifdef GSTATS_HANDLE_STATS_BROWN_EXT_IST_LF
GSTATS_HANDLE_STATS_BROWN_EXT_IST_LF(__DECLARE_STAT_ID);
#endif
#ifdef GSTATS_HANDLE_STATS_POOL_NUMA
GSTATS_HANDLE_STATS_POOL_NUMA(__DECLARE_STAT_ID);
#endif
#ifdef GSTATS_HANDLE_STATS_RECLAIMERS_WITH_EPOCHS
GSTATS_HANDLE_STATS_RECLAIMERS_WITH_EPOCHS(__DECLARE_STAT_ID);
#endif
#ifdef GSTATS_HANDLE_STATS_USER
GSTATS_HANDLE_STATS_USER(__DECLARE_STAT_ID);
#endif
// Create storage for the CONTENTS of gstats counters (for MAX_THREADS_POW2 threads)
GSTATS_DECLARE_STATS_OBJECT(MAX_THREADS_POW2);
// Create storage for the IDs of all global counters defined in define_global_statistics.h

#define TIMING_START(s) \
    std::cout<<"timing_start "<<s<<"..."<<std::endl; \
    GSTATS_TIMER_RESET(tid, timer_duration);
#define TIMING_STOP \
    std::cout<<"timing_elapsed "<<(GSTATS_TIMER_SPLIT(tid, timer_duration)/1000000000.)<<"s"<<std::endl;
#ifndef OPS_BETWEEN_TIME_CHECKS
#define OPS_BETWEEN_TIME_CHECKS 100
#endif
#ifndef RQS_BETWEEN_TIME_CHECKS
#define RQS_BETWEEN_TIME_CHECKS 10
#endif

#include "workloads/bench_parameters.h"
#include "workloads/thread_loops/thread_loop_impl.h"

#include "globals_t_impl.h"
#include "statistics.h"
#include "parse_argument.h"

class DsAdapterTest : public ::testing::Test {
protected:
    const size_t NUM_THREADS = 1;
    const test_type KEY_MIN = 0;
    const test_type KEY_MAX = 1000;
    VALUE_TYPE const NO_VALUE;
    
    DS_ADAPTER_T *adapter;
    Random64 rng[1];

    DsAdapterTest() : NO_VALUE(NULL) {
        rng[0].setSeed(rand());
        adapter = new DS_ADAPTER_T(NUM_THREADS, KEY_MIN,
                                    KEY_MAX, NO_VALUE,
                                    rng);
        adapter->initThread(0); 
    }

    // void SetUp() override {
    //     rng[0].setSeed(rand());
    //     adapter = new DS_ADAPTER_T(NUM_THREADS, KEY_MIN,
    //                                 KEY_MAX, NO_VALUE,
    //                                 rng);
    //     adapter->initThread(0); 
    // }

    // void TearDown() override {
    //     adapter->deinitThread(0);
    //     delete adapter;
    // }

    ~DsAdapterTest() {
        adapter->deinitThread(0);
        delete adapter;
    }
};

#define TEST_ADAPTER(NAME, FUNCTION_NAME) TEST_F(DsAdapterTest, NAME ##_ FUNCTION_NAME)

TEST_F(DsAdapterTest, InsertFind) {
    long long key = 10;
    long long value = 20;
    
    VALUE_TYPE result = adapter->insertIfAbsent(0, key, KEY_TO_VALUE(value));
    EXPECT_EQ(result, NO_VALUE);
    
    auto found = adapter->find(0, key);
    EXPECT_EQ(found, KEY_TO_VALUE(value));
    
    EXPECT_TRUE(adapter->contains(0, key));
}

TEST_F(DsAdapterTest, InsertIfAbsentTwice) {
    int key = 5;
    int value1 = 10;
    int value2 = 20;
    
    auto result1 = adapter->insertIfAbsent(0, key, KEY_TO_VALUE(value1));
    EXPECT_EQ(result1, NO_VALUE);
    
    auto result2 = adapter->insertIfAbsent(0, key, KEY_TO_VALUE(value2));
    EXPECT_EQ(result2, KEY_TO_VALUE(value1));
    
    EXPECT_EQ(adapter->find(0, key), KEY_TO_VALUE(value1));
}

TEST_F(DsAdapterTest, EraseTest) {
    int key = 15;
    int value = 30;
    
    adapter->insertIfAbsent(0, key, KEY_TO_VALUE(value));
    
    auto erased = adapter->erase(0, key);
    EXPECT_EQ(erased, KEY_TO_VALUE(value));
    
    EXPECT_FALSE(adapter->contains(0, key));
    EXPECT_EQ(adapter->find(0, key), NO_VALUE);
}

TEST_F(DsAdapterTest, ValidateStructure) {
    int value1 = 10;
    int value2 = 20;
    int value3 = 30;
    adapter->insertIfAbsent(0, 1, KEY_TO_VALUE(value1));
    adapter->insertIfAbsent(0, 2, KEY_TO_VALUE(value2));
    adapter->insertIfAbsent(0, 3, KEY_TO_VALUE(value3));
    
    EXPECT_TRUE(adapter->validateStructure());
    
    adapter->erase(0, 2);
    EXPECT_TRUE(adapter->validateStructure());
}

TEST_F(DsAdapterTest, EmptyOperations) {
    EXPECT_EQ(adapter->find(0, 999), NO_VALUE);
    EXPECT_FALSE(adapter->contains(0, 999));
    
    EXPECT_EQ(adapter->erase(0, 999), NO_VALUE);
}

TEST_F(DsAdapterTest, GetStats) {
    int value1 = 10;
    int value2 = 20;
    int value3 = 30;
    adapter->insertIfAbsent(0, 1, KEY_TO_VALUE(value1));
    adapter->insertIfAbsent(0, 2, KEY_TO_VALUE(value2));
    adapter->insertIfAbsent(0, 3, KEY_TO_VALUE(value3));
    
    //int height = adapter->getHeight();
    //EXPECT_GT(height, 0);
    
    // long long pathsLength = adapter->getPathsLength(0);
    // EXPECT_GT(pathsLength, 0);
}

#ifdef USE_TREE_STATS
TEST_F(DsAdapterTest, TreeStats) {
    adapter->insertIfAbsent(0, 1, 10);
    adapter->insertIfAbsent(0, 2, 20);
    adapter->insertIfAbsent(0, 3, 30);
    
    auto stats = adapter->createTreeStats(KEY_MIN, KEY_MAX);
    
    EXPECT_GE(stats->getNumKeys(), 3);
    
    delete stats;
}
#endif