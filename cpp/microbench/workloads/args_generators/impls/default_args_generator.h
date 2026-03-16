//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

using KeyType = int64_t;

class DefaultArgsGenerator : public ArgsGenerator {
private:
    DistributionPtr distribution_;
    DataMapPtr data_;

    KeyType next() {
        size_t index = distribution_->next();
        return data_->get(index);
    }

public:
    DefaultArgsGenerator(DataMapPtr data, DistributionPtr distribution)
        : data_(std::move(data)),
          distribution_(std::move(distribution)) {
    }

    KeyType next_get() override {
        return next();
    }

    KeyType next_insert() override {
        return next();
    }

    KeyType next_remove() override {
        return next();
    }

    std::pair<KeyType, KeyType> next_range() override {
        KeyType left = next_get();
        KeyType right = next_get();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~DefaultArgsGenerator() override = default;
};

}  // namespace microbench::workload
