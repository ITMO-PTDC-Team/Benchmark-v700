//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

template <typename K>
class DefaultArgsGenerator : public ArgsGenerator<K> {
private:
    //    PAD;
    Distribution* distribution_;
    DataMap<K>* data_;
    //    PAD;

    K next() {
        size_t index = distribution_->next();
        return data_->get(index);
    }

public:
    DefaultArgsGenerator(DataMap<K>* data, Distribution* distribution)
        : data_(data),
          distribution_(distribution) {
    }

    K next_get() {
        return next();
    }

    K next_insert() {
        return next();
    }

    K next_remove() {
        return next();
    }

    std::pair<K, K> next_range() {
        K left = next_get();
        K right = next_get();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~DefaultArgsGenerator() {
        delete distribution_;
        delete data_;
    }
};

}  // namespace microbench::workload
