//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "errors.h"
#include "workloads/data_maps/data_map.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

using KeyType = int64_t;

class SkewedInsertArgsGenerator : public ArgsGenerator {
    size_t skewed_length_;
    size_t inserted_number_;
    DistributionPtr distribution_;
    DataMapPtr data_map_;

public:
    SkewedInsertArgsGenerator(size_t skewed_length, DistributionPtr distribution,
                              DataMapPtr data_map)
        : inserted_number_(0),
          skewed_length_(skewed_length),
          distribution_(std::move(distribution)),
          data_map_(std::move(data_map)) {
    }

    KeyType next_get() override{setbench_error("Unsupported operation -- nextGet")}

    KeyType next_insert() override {
        KeyType value;
        if (inserted_number_ < skewed_length_) {
            value = data_map_->get(inserted_number_++);
        } else {
            value = data_map_->get(skewed_length_ + distribution_->next());
        }
        return value;
    }

    KeyType next_remove() override{setbench_error("Unsupported operation -- nextGet")}

    std::pair<KeyType, KeyType> next_range() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    ~SkewedInsertArgsGenerator() override = default;
};

}  // namespace microbench::workload
