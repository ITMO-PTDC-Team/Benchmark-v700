//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <functional>
#include <vector>
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

class ArrayDataMap : public DataMap {
private:
    std::reference_wrapper<const std::vector<int64_t>> data_;

public:
    explicit ArrayDataMap(const std::vector<int64_t>& data)
        : data_(data) {
    }

    int64_t get(size_t index) override {
        return data_.get()[index];
    }

    ~ArrayDataMap() override = default;
};

}  // namespace microbench::workload
