//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <utility>

namespace microbench::workload {

template <typename K>
struct ArgsGenerator {
    virtual K next_get() = 0;

    virtual K next_insert() = 0;

    virtual K next_remove() = 0;

    virtual std::pair<K, K> next_range() = 0;

    virtual ~ArgsGenerator() = default;
};

}  // namespace microbench::workload
