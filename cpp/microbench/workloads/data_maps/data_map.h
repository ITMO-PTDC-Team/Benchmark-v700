//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <cstddef>
#include "args_generators/args_generator.h"

namespace microbench::workload {

using KeyType = int64_t;

struct DataMap {
    virtual KeyType get(size_t index) = 0;

    virtual ~DataMap() = default;
};

using DataMapPtr = std::shared_ptr<DataMap>;

}  // namespace microbench::workload
