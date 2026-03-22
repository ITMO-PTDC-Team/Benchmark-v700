#pragma once

#include "errors.h"
#include "workloads/args_generators/args_generator.h"

namespace microbench::workload {

using KeyType = int64_t;

class NullArgsGenerator : public ArgsGenerator {
public:
    NullArgsGenerator() = default;

    KeyType next_get() {
        setbench_error("Operation not supported");
    }

    KeyType next_insert() {
        setbench_error("Operation not supported");
    }

    KeyType next_remove() {
        setbench_error("Operation not supported");
    }

    std::pair<KeyType, KeyType> next_range() {
        setbench_error("Operation not supported");
    }

    ~NullArgsGenerator() = default;
};

}  // namespace microbench::workload
