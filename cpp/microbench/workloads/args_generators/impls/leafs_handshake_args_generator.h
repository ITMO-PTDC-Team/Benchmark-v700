//
// Created by Ravil Galiev on 20.09.2023.
//
#pragma once

#include <atomic>
#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

template <typename K>
class LeafsHandshakeArgsGenerator : public ArgsGenerator<K> {
    size_t range_;

    Distribution* read_distribution_;
    MutableDistribution* insert_distribution_;
    Distribution* remove_distribution_;
    Random64& rng_;
    PAD;
    std::atomic<size_t>* deleted_value_;
    PAD;

    DataMap<K>* read_data_;
    DataMap<K>* remove_data_;

public:
    LeafsHandshakeArgsGenerator(Random64& rng, size_t range, std::atomic<size_t>* deleted_value,
                                Distribution* read_distribution,
                                MutableDistribution* insert_distribution,
                                Distribution* remove_distribution, DataMap<K>* read_data,
                                DataMap<K>* remove_data)
        : range_(range),
          read_distribution_(read_distribution),
          insert_distribution_(insert_distribution),
          remove_distribution_(remove_distribution),
          rng_(rng),
          deleted_value_(deleted_value),
          read_data_(read_data),
          remove_data_(remove_data) {
    }

    K next_get() {
        return read_data_->get(read_distribution_->next());
    }

    K next_insert() {
        size_t local_deleted_value = *deleted_value_;

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

    K next_remove() {
        size_t local_deleted_value = *deleted_value_;
        size_t value = remove_data_->get(remove_distribution_->next());

        // todo learn the difference between all kinds of weakCompareAndSet
        deleted_value_->compare_exchange_weak(local_deleted_value, value);

        return value;
    }

    std::pair<K, K> next_range() {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~LeafsHandshakeArgsGenerator() {
        delete read_data_;
        delete remove_data_;
        delete read_distribution_;
        delete insert_distribution_;
        delete remove_distribution_;
    }
};

}  // namespace microbench::workload
