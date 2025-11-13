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
  sval_t val; 
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
            curr = curr->next.load();
            delete temp;
        }
    }

    sval_t find(skey_t key) {
        mstack_node* curr = top.load(memory_order_acquire);
        while (curr != nullptr) {
            if (curr->key == key) {
                return curr->val;
            }
            curr = curr->next.load(memory_order_acquire);
        }
        return 0;
    }

    bool push(skey_t key, sval_t val) {
        mstack_node* new_node = new mstack_node(key, val);
        mstack_node* expected = top.load(memory_order_relaxed);
        
        do {
            new_node->next.store(expected, memory_order_relaxed);
        } while (!top.compare_exchange_weak(
            expected, 
            new_node,
            memory_order_release,
            memory_order_relaxed
        ));
        
        return true;
    }

    sval_t pop() {
        mstack_node* expected = top.load(memory_order_acquire);
        mstack_node* new_top;
        
        do {
            if (expected == nullptr) {
                return 0;
            }
            new_top = expected->next.load(memory_order_relaxed);
        } while (!top.compare_exchange_weak(
            expected,
            new_top,
            memory_order_release,
            memory_order_acquire));
        
        sval_t result = expected->val;
        delete expected;
        return result;
    }

    // Удаление конструкторов копирования и присваивания
    mstack(const mstack&) = delete;
    mstack& operator=(const mstack&) = delete;
};