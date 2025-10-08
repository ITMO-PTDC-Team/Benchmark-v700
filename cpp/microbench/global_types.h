#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#ifdef REDIS
    typedef KEY_TYPE VALUE_TYPE;
#else
    typedef void * VALUE_TYPE;
#endif

#ifdef USE_STRING
    #define KEY_TYPE std::string
#endif

#ifdef USE_LONG_LONG
    #define KEY_TYPE long long
#endif

#ifndef KEY_TYPE
    #define KEY_TYPE long long
#endif

#endif //SETBENCH_GLOBALS_T_H