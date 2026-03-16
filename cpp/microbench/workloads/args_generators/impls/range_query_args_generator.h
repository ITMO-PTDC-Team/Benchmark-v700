#pragma once

#include "errors.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

using KeyType = int64_t;

class RangeQueryArgsGenerator : public ArgsGenerator {
private:
    //    PAD;
    DistributionPtr distribution_;
    DataMapPtr data_;
    size_t interval_;
    //    PAD;

public:
    RangeQueryArgsGenerator(DataMapPtr data, DistributionPtr distribution, size_t interval)
        : data_(std::move(data)),
          distribution_(std::move(distribution)),
          interval_(interval) {
    }

    KeyType next_get() override {
        setbench_error("Operation not supported");
    }

    KeyType next_insert() override {
        setbench_error("Operation not supported");
    }

    KeyType next_remove() override {
        setbench_error("Operation not supported");
    }

    std::pair<KeyType, KeyType> next_range() override {
        size_t index = distribution_->next();
        KeyType left = data_->get(index);
        KeyType right = data_->get(index + interval_);
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~RangeQueryArgsGenerator() override = default;
};

}  // namespace microbench::workload
