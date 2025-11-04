//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <cstddef>

namespace microbench::workload {

template <typename K>
struct DataMap {
    virtual K get(size_t index) = 0;

    virtual ~DataMap() = default;
};

}  // namespace microbench::workload
