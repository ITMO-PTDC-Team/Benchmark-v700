#pragma once

#include <cstdint>
#include <atomic>
#include <memory>

using namespace std;

typedef intptr_t skey_t;
typedef intptr_t sval_t;

#define CACHE_LINE_SIZE 64

struct queue_node
{
    skey_t key;
    atomic<queue_node*> next;
    
    queue_node(skey_t k, sval_t v) : key(k), next(nullptr) {}
    queue_node() : key(0), next(nullptr) {}
};

struct alignas(CACHE_LINE_SIZE) michael_scott_queue
{
    atomic<queue_node*> head;
    atomic<queue_node*> tail;

    michael_scott_queue() {
        queue_node* dummy = new queue_node();
        head.store(dummy, memory_order_relaxed);
        tail.store(dummy, memory_order_relaxed);
    }
    
    ~michael_scott_queue() {
        queue_node* curr = head.load();
        while (curr != nullptr) {
            queue_node* temp = curr;
            curr = curr->next.load();
            delete temp;
        }
    }

    bool enqueue(skey_t key) {
        queue_node* new_node = new queue_node(key);
        
        while (true) {
            queue_node* last = tail.load(memory_order_acquire);
            queue_node* next = last->next.load(memory_order_acquire);
            
            if (last == tail.load(memory_order_relaxed)) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(
                        next, new_node, memory_order_release, memory_order_relaxed)) {
                        tail.compare_exchange_weak(
                            last, new_node, memory_order_release, memory_order_relaxed);
                        return true;
                    }
                } else {
                    tail.compare_exchange_weak(
                        last, next, memory_order_release, memory_order_relaxed);
                }
            }
        }
    }

    bool dequeue(skey_t& key) {
        while (true) {
            queue_node* first = head.load(memory_order_acquire);
            queue_node* last = tail.load(memory_order_acquire);
            queue_node* next = first->next.load(memory_order_acquire);
            
            if (first == head.load(memory_order_relaxed)) {
                if (first == last) {
                    if (next == nullptr) {
                        return false; 
                    }
                    tail.compare_exchange_weak(
                        last, next, memory_order_release, memory_order_relaxed);
                } else {
                    key = next->key;
                    
                    if (head.compare_exchange_weak(
                        first, next, memory_order_release, memory_order_relaxed)) {
                        delete first;
                        return true;
                    }
                }
            }
        }
    }

    bool is_empty() {
        queue_node* first = head.load(memory_order_acquire);
        queue_node* next = first->next.load(memory_order_acquire);
        return (next == nullptr);
    }

    michael_scott_queue(const michael_scott_queue&) = delete;
    michael_scott_queue& operator=(const michael_scott_queue&) = delete;
};