//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace microbench::workload {

using KeyType = int64_t;

struct ArgsGenerator {
    virtual KeyType next_get() = 0;

    virtual KeyType next_insert() = 0;

    virtual KeyType next_remove() = 0;

    virtual std::pair<KeyType, KeyType> next_range() = 0;

    virtual ~ArgsGenerator() = default;
};

using ArgsGeneratorPtr = std::shared_ptr<ArgsGenerator>;


}  // namespace microbench::workload
