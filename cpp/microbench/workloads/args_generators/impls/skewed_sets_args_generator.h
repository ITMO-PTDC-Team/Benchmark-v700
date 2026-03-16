//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "workloads/data_maps/data_map.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

using KeyType = int64_t;

class SkewedSetsArgsGenerator : public ArgsGenerator {
    //    PAD;
    size_t range_;
    size_t write_set_begins_;
    DistributionPtr read_dist_;
    DistributionPtr write_dist_;
    DataMapPtr data_map_;

    KeyType next_write() {
        size_t index = write_set_begins_ + write_dist_->next();
        if (index >= range_) {
            index -= range_;
        }
        return data_map_->get(index);
    }

public:
    SkewedSetsArgsGenerator(size_t range, size_t write_set_begins, DistributionPtr read_dist,
                            DistributionPtr write_dist, DataMapPtr data_map)
        : range_(range),
          write_set_begins_(write_set_begins),
          read_dist_(std::move(read_dist)),
          write_dist_(std::move(write_dist)),
          data_map_(std::move(data_map)) {
    }

    KeyType next_get() override {
        return data_map_->get(read_dist_->next());
    }

    KeyType next_insert() override {
        return next_write();
    }

    KeyType next_remove() override {
        return next_write();
    }

    std::pair<KeyType, KeyType> next_range() override {
        KeyType left = next_get();
        KeyType right = next_get();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~SkewedSetsArgsGenerator() override = default;
};

}  // namespace microbench::workload
