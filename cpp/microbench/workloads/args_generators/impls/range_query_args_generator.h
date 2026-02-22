#pragma once

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

template <typename K>
class RangeQueryArgsGenerator : public ArgsGenerator<K> {
private:
    //    PAD;
    Distribution* distribution_;
    DataMap<K>* data_;
    size_t interval_;
    //    PAD;

public:
    RangeQueryArgsGenerator(DataMap<K>* data, Distribution* distribution, size_t interval)
        : data_(data),
          distribution_(distribution),
          interval_(interval) {
    }

    K next_get() {
        setbench_error("Operation not supported");
    }

    K next_insert() {
        setbench_error("Operation not supported");
    }

    K next_remove() {
        setbench_error("Operation not supported");
    }

    std::pair<K, K> next_range() {
        size_t index = distribution_->next();
        K left = data_->get(index);
        K right = data_->get(index + interval_);
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~RangeQueryArgsGenerator() {
        delete distribution_;
        delete data_;
    }
};

}  // namespace microbench::workload
