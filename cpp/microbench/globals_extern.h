/*
 * File:   globals_extern.h
 * Author: trbot
 *
 * Created on March 9, 2015, 1:32 PM
 */
#pragma once

// enable USE_TRACE if you want low level functionality tracing using std::cout
// #define USE_TRACE

#include <string>

#ifdef MAIN_BENCH

#include "debugprinting.h"
#include "plaf.h"
#include <atomic>

#ifndef __rtm_force_inline
#define __rtm_force_inline __attribute__((always_inline)) inline
#endif

#ifndef DEBUG
#define DEBUG if (0)
#define DEBUG1 if (0)
#define DEBUG2 if (0)
#define DEBUG3 if (0) /* rarely used */
#endif

#ifdef __unix__
#define POSIX_SYSTEM
#else
#error NOT UNIX SYSTEM
#endif

#ifndef TRACE
#ifdef USE_TRACE
extern std::atomic_bool ___trace;
#define TRACE_TOGGLE          \
    {                         \
        bool ___t = ___trace; \
        ___trace = !___t;     \
    }
#define TRACE_ON         \
    {                    \
        ___trace = true; \
    }
#define TRACE DEBUG if (___trace)
extern std::atomic_bool ___validateops;
#define VALIDATEOPS_ON         \
    {                          \
        ___validateops = true; \
    }
#define VALIDATEOPS DEBUG if (___validateops)
#endif
#endif

/**
 * Enable global statistics access using gstats_global.h
 */

#include "gstats_global.h"

/**
 * Setup timing code
 */

#include "server_clock.h"

/**
 * Configure record manager: reclaimer, allocator and pool
 */

#ifndef PRINTS
#define STR(x) XSTR(x)
#define XSTR(x) #x
#define PRINTI(name)                                    \
    {                                                   \
        std::cout << #name << "=" << name << std::endl; \
    }
#define PRINTS(name)                                         \
    {                                                        \
        std::cout << #name << "=" << STR(name) << std::endl; \
    }
#endif

#define _EVAL(a) a
#define _PASTE2(a, b) a##b
#define PASTE(a, b) _PASTE2(a, b)
#ifdef RECLAIM_TYPE
#define RECLAIM PASTE(reclaimer_, RECLAIM_TYPE)
// #define RECLAIM_DOT_H PASTE(RECLAIM,.h)
#include STR(RECLAIM.h)
#else
#define RECLAIM reclaimer_debra

#include "reclaimer_debra.h"

#endif
#ifdef ALLOC_TYPE
#define ALLOC PASTE(allocator_, ALLOC_TYPE)
// #define ALLOC_DOT_H PASTE(ALLOC,.h)
#include STR(ALLOC.h)
#else
#define ALLOC allocator_new

#include "allocator_new.h"

#endif
#ifdef POOL_TYPE
#define POOL PASTE(pool_, POOL_TYPE)
// #define POOL_DOT_H PASTE(POOL,.h)
#include STR(POOL.h)
#else
#define POOL pool_none

#include "pool_none.h"

#endif

#endif

std::string indented_title(const std::string& title, size_t indents = 1, size_t indent_length = 2) {
    return std::string(indents * indent_length, ' ') + title + ":\n";
}

template <typename T>
std::string indented_title_with_data(const std::string& title, const T& data, size_t indents = 1,
                                     size_t line_length = 28, size_t indent_length = 2) {
    return std::string(indents * indent_length, ' ') + title + ":" +
           (title.size() + indents * indent_length < line_length
                ? std::string(line_length - title.size() - indents * indent_length, ' ')
                : "\t") +
           std::to_string(data) + "\n";
}

std::string indented_title_with_str_data(const std::string& title, const std::string& data,
                                         size_t indents = 1, size_t line_length = 28,
                                         size_t indent_length = 2) {
    return std::string(indents * indent_length, ' ') + title + ":" +
           (title.size() + indents * indent_length < line_length
                ? std::string(line_length - title.size() - indents * indent_length, ' ')
                : "\t") +
           data + "\n";
}

std::string to_string_stage(const std::string& stage_name) {
    return std::string(80, '-') + "\n" + stage_name + "\n" + std::string(80, '-') + "\n";
}

std::string to_string_big_stage(const std::string& stage_name) {
    size_t size = (78 - stage_name.size()) / 2;
    return std::string(80, '#') + "\n" +
           std::string(78 - size * 2 - stage_name.size() == 0 ? size : size + 1, '#') + ' ' +
           stage_name + ' ' + std::string(size, '#') + "\n" + std::string(80, '#') + "\n";
}
