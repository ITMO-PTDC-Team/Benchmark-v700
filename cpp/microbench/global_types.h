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

#ifdef USE_STRING
    KEY_TYPE get_min() {
        return "a";
    }

    KEY_TYPE get_max(size_t range) {
        std::string mx_val;
        for (int i = 0; i<range; ++i) {
            mx_val += "a";
        }
        return mx_val;
    }
#endif

#ifdef USE_LONG_LONG
    KEY_TYPE get_min() {
        return 0;
    }

    KEY_TYPE get_max(size_t range) {
        return range + 1;
    }
#endif

#endif //SETBENCH_GLOBALS_T_H