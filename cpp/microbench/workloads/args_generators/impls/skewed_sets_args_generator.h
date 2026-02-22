//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"
#include "workloads/data_maps/data_map.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

template <typename K>
class SkewedSetsArgsGenerator : public ArgsGenerator<K> {
    //    PAD;
    size_t range_;
    size_t write_set_begins_;
    Distribution* read_dist_;
    Distribution* write_dist_;
    DataMap<K>* data_map_;

    K next_write() {
        size_t index = write_set_begins_ + write_dist_->next();
        if (index >= range_) {
            index -= range_;
        }
        return data_map_->get(index);
    }

public:
    SkewedSetsArgsGenerator(size_t range, size_t write_set_begins, Distribution* read_dist,
                            Distribution* write_dist, DataMap<K>* data_map)
        : range_(range),
          write_set_begins_(write_set_begins),
          read_dist_(read_dist),
          write_dist_(write_dist),
          data_map_(data_map) {
    }

    K next_get() override {
        return data_map_->get(read_dist_->next());
    }

    K next_insert() override {
        return next_write();
    }

    K next_remove() override {
        return next_write();
    }

    std::pair<K, K> next_range() override {
        K left = next_get();
        K right = next_get();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~SkewedSetsArgsGenerator() override {
        delete read_dist_;
        delete write_dist_;
        delete data_map_;
    };
};

}  // namespace microbench::workload
