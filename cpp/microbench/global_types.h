#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#ifndef KEY_TYPE
    #define KEY_TYPE long long
    #define USE_LONG_LONG
    // #define USE_STRING
#endif

#ifdef REDIS
    typedef KEY_TYPE VALUE_TYPE;
#else
    typedef void * VALUE_TYPE;
#endif

#endif //SETBENCH_GLOBALS_T_H