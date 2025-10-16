//
// Created by Ravil Galiev on 26.07.2023.
//
#ifndef SETBENCH_GLOBALS_T_H
#define SETBENCH_GLOBALS_T_H

#include "global_types.h"

#ifdef REDIS
    #define VALUE_TYPE test_type
    #define KEY_TO_VALUE(key) key
    #define GET_FUNC executeContains
#else
    #define KEY_TO_VALUE(key) &key /* note: hack to turn a key into a pointer */
    #define GET_FUNC executeGet
#endif

#define DS_ADAPTER_T ds_adapter<KEY_TYPE, VALUE_TYPE, RECLAIM<>, ALLOC<>, POOL<> >

struct globals_t;

#endif //SETBENCH_GLOBALS_T_H
