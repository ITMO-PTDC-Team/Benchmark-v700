//
// Created by Ravil Galiev on 26.07.2023.
//
#pragma once

#include <cstdint>
typedef int64_t test_type;

#ifdef REDIS
#define VALUE_TYPE test_type
#define KEY_TO_VALUE(key) key
#define GET_FUNC execute_contains
#else
#define VALUE_TYPE void*
#define KEY_TO_VALUE(key) &key /* note: hack to turn a key into a pointer */
#define GET_FUNC execute_get
#endif

#define DS_ADAPTER_T ds_adapter<test_type, VALUE_TYPE, RECLAIM<>, ALLOC<>, POOL<> >

struct globals_t;
