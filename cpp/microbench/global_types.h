#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

// typedef std::string KEY_TYPE;

#ifndef KEY_TYPE
    #define KEY_TYPE long long
#endif

// #if ACTUAL_KEY_TYPE 
// struct KEY_TYPE {
    // ACTUAL_KEY_TYPE value;

    // KEY_TYPE() = default;

    // template <typename T>
    // KEY_TYPE(T&& val) {
    //     value = std::forward<T>(val);
    // }

    // KEY_TYPE(const KEY_TYPE& oth) {
    //     value = oth.value;
    // }

    // KEY_TYPE(KEY_TYPE&& oth) {
    //     value = oth.value;
    // }

    // volatile KEY_TYPE& self() volatile { return *this; }

    // KEY_TYPE& operator=(const KEY_TYPE& oth) {
    //     value = oth.value;
    //     return *this;
    // }

    // volatile KEY_TYPE& operator=(const KEY_TYPE& oth) volatile { 
    //     value = oth.value;
    //     return self();
    // }

    // KEY_TYPE& operator=(KEY_TYPE&& oth) {
    //     value = oth.value;
    //     return *this;
    // }

    // // conversion operators
    // operator long long() const {
    //     return value;
    // }

    // ACTUAL_KEY_TYPE operator()() const {
    //     return value;
    // }

    // // add operators
    // KEY_TYPE& operator+=(const KEY_TYPE& oth) {
    //     value += oth.value;
    //     return *this;
    // }

    // volatile KEY_TYPE operator+=(const KEY_TYPE& oth) volatile { 
    //     value += oth.value;
    //     return *this;  
    // }   
// };


#ifdef REDIS
    typedef KEY_TYPE VALUE_TYPE;
#else
    typedef void * VALUE_TYPE;
#endif

#endif //SETBENCH_GLOBALS_T_H