// coroutine_thread_wrapper.h
#pragma once

#include <boost/coroutine2/all.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <iostream>

using coro_yield = boost::coroutines2::coroutine<void>::push_type;
using coro_source = boost::coroutines2::coroutine<void>::pull_type;

template<typename K>
class ThreadCoroutineManager {
private:
    static thread_local coro_yield* thread_yield_ptr;
    
public:
    static void set_yield(coro_yield& yield) {
        thread_yield_ptr = &yield;
    }
    
    static void clear_yield() {
        thread_yield_ptr = nullptr;
    }
    
    static void yield_if_needed() {
        if (thread_yield_ptr != nullptr) {
            (*thread_yield_ptr)();
        }
    }
    
    static bool has_yield() {
        return thread_yield_ptr != nullptr;
    }
};

template<typename K>
thread_local coro_yield* ThreadCoroutineManager<K>::thread_yield_ptr = nullptr;