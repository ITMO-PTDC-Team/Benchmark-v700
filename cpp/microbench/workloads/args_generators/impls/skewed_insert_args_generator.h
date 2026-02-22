//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"
#include "errors.h"
#include "workloads/data_maps/data_map.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

template <typename K>
class SkewedInsertArgsGenerator : public ArgsGenerator<K> {
    //    PAD;
    size_t skewed_length_;
    size_t inserted_number_;
    Distribution* distribution_;
    // PAD;
    DataMap<K>* data_map_;
    // PAD;

public:
    SkewedInsertArgsGenerator(size_t skewed_length, Distribution* distribution,
                              DataMap<K>* data_map)
        : inserted_number_(0),
          skewed_length_(skewed_length),
          distribution_(distribution),
          data_map_(data_map) {
    }

    K next_get() override{setbench_error("Unsupported operation -- nextGet")}

    K next_insert() override {
        K value;
        if (inserted_number_ < skewed_length_) {
            value = data_map_->get(inserted_number_++);
        } else {
            value = data_map_->get(skewed_length_ + distribution_->next());
        }
        return value;
    }

    K next_remove() override{setbench_error("Unsupported operation -- nextGet")}

    std::pair<K, K> next_range() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    ~SkewedInsertArgsGenerator() override {
        delete distribution_;
        delete data_map_;
    };
};

}  // namespace microbench::workload
