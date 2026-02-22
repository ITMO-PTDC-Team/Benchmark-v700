#pragma once

#include "workloads/args_generators/args_generator.h"

namespace microbench::workload {

template <typename K>
class NullArgsGenerator : public ArgsGenerator<K> {
public:
    NullArgsGenerator() = default;

    K next_get() {
        setbench_error("Operation not supported");
    }

    K next_insert() {
        setbench_error("Operation not supported");
    }

    K next_remove() {
        setbench_error("Operation not supported");
    }

    std::pair<K, K> next_range() {
        setbench_error("Operation not supported");
    }

    ~NullArgsGenerator() = default;
};

}  // namespace microbench::workload
