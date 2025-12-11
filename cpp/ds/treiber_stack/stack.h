/*   
 *   Updated Treiber stack from ASCYLIB
 */
#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <boost/coroutine2/all.hpp>

using namespace std;

typedef intptr_t skey_t;

#define CACHE_LINE_SIZE 64

thread_local static boost::coroutines2::coroutine<void>::push_type* thread_yield = nullptr;

template <typename K>
struct mstack_node
{
  K key;
  struct mstack_node* next;
};

template <typename K>
struct alignas(CACHE_LINE_SIZE) mstack
{
    atomic<mstack_node<K>*> top;
    coro_yield* yield_ptr = nullptr;

    mstack() : top(nullptr) {}
    
    ~mstack() {
        mstack_node<K>* curr = top.load();
        while (curr != nullptr) {
            mstack_node<K>* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    static void set_thread_yield(boost::coroutines2::coroutine<void>::push_type& yield) {
        thread_yield = &yield;
    }

    static void clear_thread_yield() {
        thread_yield = nullptr;
    }

    K find(const int tid, skey_t key) {
        // mstack_node* curr = top.load(memory_order_acquire);
        // while (curr != nullptr) {
        //     if (curr->key == key) {
        //         return curr->key;
        //     }
        //     curr = curr->next.load(memory_order_acquire);
        // }
        return nullptr;
    }

    unique_ptr<K> push(const int tid, skey_t key) {
        mstack_node<K>* new_node = new mstack_node<K>(key);
        mstack_node<K>* expected = top.load(memory_order_relaxed);
        
        do {
            new_node->next = expected;
            if (thread_yield != nullptr) {
                (*thread_yield)();
            }
        } while (!top.compare_exchange_weak(
            expected, 
            new_node,
            memory_order_release,
            memory_order_relaxed
        ));
        
        return std::make_unique<K>(key);
    }

    unique_ptr<K> pop(const int tid) {
        mstack_node<K>* expected = top.load(memory_order_acquire);
        mstack_node<K>* new_top;
        
        do {
            if (expected == nullptr) {
                return 0;
            }
            // new_top = expected->next.load(memory_order_relaxed);
            new_top = top.load(memory_order_acquire)->next;
        } while (!top.compare_exchange_weak(
            expected,
            new_top,
            memory_order_release,
            memory_order_acquire));
        
        auto result = expected->key;
        delete expected;
        return std::make_unique<K>(result);
    }

    mstack(const mstack&) = delete;
    mstack& operator=(const mstack&) = delete;
};