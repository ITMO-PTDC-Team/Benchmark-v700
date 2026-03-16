//
// Created by Ravil Galiev on 20.09.2023.
//
#pragma once

#include <atomic>
#include "errors.h"
#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

using KeyType = int64_t;

class LeafsHandshakeArgsGenerator : public ArgsGenerator {
    size_t range_;

    DistributionPtr read_distribution_;
    MutableDistributionPtr insert_distribution_;
    DistributionPtr remove_distribution_;
    Random64& rng_;
    PAD;
    std::atomic<size_t> deleted_value_;
    PAD;

    DataMapPtr read_data_;
    DataMapPtr remove_data_;

public:
    LeafsHandshakeArgsGenerator(Random64& rng, size_t range, size_t deleted_value,
                                DistributionPtr read_distribution,
                                MutableDistributionPtr insert_distribution,
                                DistributionPtr remove_distribution, DataMapPtr read_data,
                                DataMapPtr remove_data)
        : range_(range),
          read_distribution_(std::move(read_distribution)),
          insert_distribution_(std::move(insert_distribution)),
          remove_distribution_(std::move(remove_distribution)),
          rng_(rng),
          deleted_value_(deleted_value),
          read_data_(std::move(read_data)),
          remove_data_(std::move(remove_data)) {
    }

    KeyType next_get() override {
        return read_data_->get(read_distribution_->next());
    }

    KeyType next_insert() override {
        size_t local_deleted_value = deleted_value_;

        size_t value;

        bool is_right = rng_.nextDouble() >= 0.5;

        if (local_deleted_value == 1 || (is_right && local_deleted_value != range_)) {
            value =
                local_deleted_value + insert_distribution_->next(range_ - local_deleted_value) + 1;
        } else {
            value = local_deleted_value - insert_distribution_->next(local_deleted_value - 1) - 1;
        }

        return value;
    }

    KeyType next_remove() override {
        size_t local_deleted_value = deleted_value_;
        size_t value = remove_data_->get(remove_distribution_->next());

        // todo learn the difference between all kinds of weakCompareAndSet
        deleted_value_.compare_exchange_strong(local_deleted_value, value);

        return value;
    }

    std::pair<KeyType, KeyType> next_range() override {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~LeafsHandshakeArgsGenerator() override = default;
};

}  // namespace microbench::workload
