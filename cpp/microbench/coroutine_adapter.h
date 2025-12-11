// coroutine_thread_adapter.h
#pragma once

#include <thread>
#include <vector>
#include <functional>
#include <boost/coroutine2/all.hpp>

template<typename K, typename ThreadLoopType>
class CoroutineThreadAdapter {
private:
    struct ThreadContext {
        std::thread* thread;
        ThreadLoopType* thread_loop;
        boost::coroutines2::coroutine<void>::push_type* yield_ptr;
        
        ThreadContext() : thread(nullptr), thread_loop(nullptr), yield_ptr(nullptr) {}
    };
    
    std::vector<ThreadContext> contexts;
    int num_coroutine_threads;
    
public:
    CoroutineThreadAdapter(int total_threads, int coro_threads = -1) 
        : contexts(total_threads)
        , num_coroutine_threads(coro_threads == -1 ? total_threads : coro_threads) {}
    
    ~CoroutineThreadAdapter() {
        for (auto& ctx : contexts) {
            if (ctx.yield_ptr) {
                delete ctx.yield_ptr;
            }
        }
    }
    
    std::thread* create_coroutine_thread(int idx, ThreadLoopType* thread_loop) {
        contexts[idx].thread_loop = thread_loop;
        
        auto* yield_ptr = new boost::coroutines2::coroutine<void>::push_type(
            [this, idx, thread_loop](boost::coroutines2::coroutine<void>::pull_type&) {
                // local yield SET
                ThreadCoroutineManager<K>::set_yield(*contexts[idx].yield_ptr);
                thread_loop->run();
                ThreadCoroutineManager<K>::clear_yield();
            }
        );
        
        contexts[idx].yield_ptr = yield_ptr;
        contexts[idx].thread = new std::thread([this, idx, yield_ptr]() {
            boost::coroutines2::coroutine<void>::pull_type coro_pull(*yield_ptr);
            while (coro_pull) {
                coro_pull();
            }
        });
        
        return contexts[idx].thread;
    }
    std::thread* create_normal_thread(int idx, ThreadLoopType* thread_loop) {
        contexts[idx].thread_loop = thread_loop;
        contexts[idx].thread = new std::thread(&ThreadLoopType::run, thread_loop);
        return contexts[idx].thread;
    }
    void yield_thread(int idx) {
        if (idx < contexts.size() && contexts[idx].yield_ptr) {
            (*contexts[idx].yield_ptr)();
        }
    }
    void join_all() {
        for (auto& ctx : contexts) {
            if (ctx.thread && ctx.thread->joinable()) {
                ctx.thread->join();
            }
        }
    }
};