//
// Created by Ravil Galiev on 26.07.2023.
//
#pragma once
#include <cstdint>
#include "adapter.h"
#include "allocator_new.h"
#include "pool_none.h"
#include "reclaimer_debra.h"

namespace microbench {

using test_type = int64_t;

#ifdef REDIS
#define VALUE_TYPE test_type
#define KEY_TO_VALUE(key) key
#define GET_FUNC execute_contains
#else
#define VALUE_TYPE void*
#define KEY_TO_VALUE(key) &key /* note: hack to turn a key into a pointer */
#define GET_FUNC execute_get
#endif

using DS_ADAPTER_T = ds_adapter<test_type, VALUE_TYPE, reclaimer_debra<>, allocator_new<>, pool_none<>>;

struct globals_t;

}  // namespace microbench
