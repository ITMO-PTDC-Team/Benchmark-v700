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
  uint8_t padding[CACHE_LINE_SIZE - sizeof(skey_t) - sizeof(sval_t) - sizeof(struct mstack_node*)];
};

struct alignas(CACHE_LINE_SIZE) mstack
{
    atomic<mstack_node*> top;
    /* ptlock_t head_lock; */
    /* uint8_t padding1[CACHE_LINE_SIZE - sizeof(mstack_node*) - sizeof(ptlock_t)]; */
    /* mstack_node* tail; */
    /* ptlock_t tail_lock; */
    uint8_t padding2[CACHE_LINE_SIZE - sizeof(mstack_node*)];

    sval_t mstackreiber_find(skey_t key);
    int mstackreiber_push(skey_t key, sval_t val);
    sval_t mstackreiber_pop();
};

int floor_log_2(unsigned int n);

/* 
 * Create a new node without setting its next fields. 
 */
mstack_node* mstack_new_simple_node(skey_t key, sval_t val, int toplevel, int transactional);
/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
mstack_node *mstack_new_node(skey_t key, sval_t val, mstack_node *next);
void mstack_delete_node(mstack_node* n);
mstack* mstack_new();
void mstack_delete(mstack* qu);
int mstack_size(mstack* cqu);

// RETRY_STATS_VARS;
// #if LATENCY_PARSING == 1
// __thread size_t lat_parsing_get = 0;
// __thread size_t lat_parsing_put = 0;
// __thread size_t lat_parsing_rem = 0;
// #endif	/* LATENCY_PARSING == 1 */

// extern __thread unsigned long* seeds;

sval_t mstack::mstackreiber_find(skey_t key)
{ 
  return 1;
}

int mstack::mstackreiber_push(skey_t key, sval_t val)
{
//   NUM_RETRIES();
  mstack_node* node = mstack_new_node(key, val, NULL);
  while(1)
    {
      mstack_node* top = qu->top;
      node->next = top;
      if (CAS_PTR(&qu->top, top, node) == top)
	{
	  break;
	}
      DO_PAUSE();
    }
  return 1;
}


sval_t mstack::mstackreiber_pop()
{
  mstack_node* mem_top;
//   NUM_RETRIES();
  while (1)
    {
      mem_top = this->top;
      if (unlikely(top == NULL))
	{
	  return 0;
	}

      if (compares(&qu->top, top, top->next) == top)
	{
	  break;
	}

      DO_PAUSE();
    }

#if GC == 1
  ssmem_free(alloc, (void*) top);
#endif
  return top->val;
}