/*   
 *   Updated Treiber stack from ASCYLIB
 */
#pragma once

#include <cstdint>
#include <atomic>
#include <memory>

using namespace std;

typedef intptr_t skey_t;

#define CACHE_LINE_SIZE 64

template <typename K>
struct mstack_node
{
  K key;
  struct mstack_node* next;

  explicit mstack_node(K k) : key(k), next(nullptr) {}
};

enum class RequestType {
    NONE,
    PUSH,
    POP,
    FIND,
    EMPTY
};

template <typename K>
struct alignas(CACHE_LINE_SIZE) FCRequest {
    atomic<RequestType> req_type;
    atomic<bool> completed;
    
    skey_t key;
    
    unique_ptr<K> result;
    bool bool_result;
    
    FCRequest() : req_type(RequestType::NONE), completed(false), 
                 key(0), bool_result(false) {}
};

template <typename K>
struct alignas(CACHE_LINE_SIZE) mstack
{
    mstack_node<K>* top;
    
    atomic<int> combiner_lock;
    static constexpr int MAX_THREADS = 32;
    FCRequest<K> op_records[MAX_THREADS];
    
    static constexpr int FC_PASSES = 2;

    mstack() : top(nullptr), combiner_lock(0) {}
    
    ~mstack() {
        mstack_node<K>* curr = top;
        while (curr != nullptr) {
            mstack_node<K>* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

private:
    bool try_lock() {
        int expected = 0;
        return combiner_lock.compare_exchange_strong(
            expected, 1, 
            memory_order_acquire, 
            memory_order_relaxed
        );
    }
    
    void unlock() {
        combiner_lock.store(0, memory_order_release);
    }
    
    unique_ptr<K> sequential_push(skey_t key) {
        mstack_node<K>* new_node = new mstack_node<K>(key);
        new_node->next = top;
        top = new_node;
        return std::make_unique<K>(key);
    }
    
    unique_ptr<K> sequential_pop() {
        if (top == nullptr) {
            return nullptr;
        }
        mstack_node<K>* old_top = top;
        top = top->next;
        auto result = old_top->key;
        delete old_top;
        return std::make_unique<K>(result);
    }
    
    K sequential_find(skey_t key) {
        mstack_node<K>* curr = top;
        while (curr != nullptr) {
            if (curr->key == key) {
                return curr->key;
            }
            curr = curr->next;
        }
        return K{};
    }
    
    bool sequential_empty() const {
        return top == nullptr;
    }
    
    void combine_operations() {
        for (int pass = 0; pass < FC_PASSES; ++pass) {
            for (int tid = 0; tid < MAX_THREADS; ++tid) {
                FCRequest<K>& record = op_records[tid];
                
                RequestType op = record.req_type.load(memory_order_acquire);
                
                if (op == RequestType::NONE || record.completed.load(memory_order_relaxed)) {
                    continue;
                }
                
                switch (op) {
                    case RequestType::PUSH:
                        record.result = sequential_push(record.key);
                        break;
                        
                    case RequestType::POP:
                        record.result = sequential_pop();
                        break;
                        
                    case RequestType::FIND:
                        record.result = std::make_unique<K>(sequential_find(record.key));
                        break;
                        
                    case RequestType::EMPTY:
                        record.bool_result = sequential_empty();
                        break;
                        
                    default:
                        break;
                }
                
                record.completed.store(true, memory_order_release);
            }
        }
    }
    
    template<typename ResultType>
    ResultType execute_op(const int tid, RequestType op, skey_t key = 0) {
        FCRequest<K>& record = op_records[tid];
        
        record.completed.store(false, memory_order_relaxed);
        record.key = key;
        record.req_type.store(op, memory_order_release);
        
        if (try_lock()) {
            combine_operations();
            unlock();
        }
        
        record.req_type.store(RequestType::NONE, memory_order_relaxed);
        
        if constexpr (std::is_same_v<ResultType, unique_ptr<K>>) {
            return std::move(record.result);
        } else if constexpr (std::is_same_v<ResultType, K>) {
            return record.result ? *record.result : K{};
        } else if constexpr (std::is_same_v<ResultType, bool>) {
            return record.bool_result;
        }
    }

public:
    K find(const int tid, skey_t key) {
        return execute_op<K>(tid, RequestType::FIND, key);
    }

    unique_ptr<K> push(const int tid, skey_t key) {
        return execute_op<unique_ptr<K>>(tid, RequestType::PUSH, key);
    }

    unique_ptr<K> pop(const int tid) {
        return execute_op<unique_ptr<K>>(tid, RequestType::POP);
    }

    bool empty() const {
        return top == nullptr;
    }

    mstack(const mstack&) = delete;
    mstack& operator=(const mstack&) = delete;
};