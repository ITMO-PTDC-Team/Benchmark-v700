/*   
 *   Updated Treiber stack from ASCYLIB
 */
#pragma once

#include <cstdint>
#include <atomic>

using namespace std;

typedef intptr_t skey_t;
typedef intptr_t sval_t;

#define CACHE_LINE_SIZE 64

struct mstack_node
{
  skey_t key;
  struct mstack_node* next;
};

struct alignas(CACHE_LINE_SIZE) mstack
{
    atomic<mstack_node*> top;

    mstack() : top(nullptr) {}
    
    ~mstack() {
        mstack_node* curr = top.load();
        while (curr != nullptr) {
            mstack_node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    sval_t find(const int tid, skey_t key) {
        // mstack_node* curr = top.load(memory_order_acquire);
        // while (curr != nullptr) {
        //     if (curr->key == key) {
        //         return curr->key;
        //     }
        //     curr = curr->next.load(memory_order_acquire);
        // }
        return nullptr;
    }

    sval_t push(const int tid, skey_t key) {
        mstack_node* new_node = new mstack_node(key);
        mstack_node* expected = top.load(memory_order_relaxed);
        
        do {
            new_node->next = expected;
            // co_yield
        } while (!top.compare_exchange_weak(
            expected, 
            new_node,
            memory_order_release,
            memory_order_relaxed
        ));
        
        return key;
    }

    sval_t pop(const int tid) {
        mstack_node* expected = top.load(memory_order_acquire);
        mstack_node* new_top;
        
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
        
        sval_t result = expected->key;
        delete expected;
        return result;
    }

    mstack(const mstack&) = delete;
    mstack& operator=(const mstack&) = delete;
};