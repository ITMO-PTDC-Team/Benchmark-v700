//
// Created by Ravil Galiev on 06.04.2023.
//
#pragma once

#include <cstddef>

struct ParseArgument {
    size_t pointer;
    size_t length;
    char** args;

    ParseArgument(size_t length, char** args)
        : length(length),
          args(args),
          pointer(0) {
    }

    char* get_current() {
        return args[pointer];
    }

    char* get_next() {
        return args[++pointer];
    }

    ParseArgument& next() {
        ++pointer;
        return *this;
    }

    bool has_next() {
        return pointer < length;
    }
};
