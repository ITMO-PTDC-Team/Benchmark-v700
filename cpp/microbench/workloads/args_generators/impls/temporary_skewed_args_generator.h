//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include <vector>
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

/**
    n — { xi — yi — ti — rti } // либо   n — rt — { xi — yi — ti }
        n — количество элементов
        xi — процент элементов i-ого множества
        yi — вероятность чтения элемента из i-ого множества
          // 100% - yi — чтение остальных элементов
        ti — время / количество итераций работы в режиме горячего вызова i-ого множества
        rti / rt (relax time) — время / количество итераций работы в обычном режиме (равномерное
                                распределение на все элементы)
          // rt — если relax time всегда одинаковый
          // rti — relax time после горячей работы с i-ым множеством
 */
using KeyType = int64_t;

class TemporarySkewedArgsGenerator : public ArgsGenerator {
    size_t time_;
    size_t pointer_;
    bool is_relax_time_;

    std::vector<DistributionPtr> hot_dists_;
    DistributionPtr relax_dist_;
    DataMapPtr data_map_;
    std::vector<int64_t> hot_times_;
    std::vector<int64_t> relax_times_;
    std::vector<size_t> set_begins_;
    size_t set_number_;
    size_t range_;

    void update_pointer() {
        if (!is_relax_time_) {
            if (time_ >= hot_times_[pointer_]) {
                time_ = 0;
                is_relax_time_ = true;
            }
        } else {
            if (time_ >= relax_times_[pointer_]) {
                time_ = 0;
                is_relax_time_ = false;
                ++pointer_;
                if (pointer_ >= set_number_) {
                    pointer_ = 0;
                }
            }
        }
        ++time_;
    }

    KeyType next() {
        update_pointer();
        KeyType value;

        if (is_relax_time_) {
            value = data_map_->get(relax_dist_->next());
        } else {
            size_t index = set_begins_[pointer_] + hot_dists_[pointer_]->next();
            if (index >= range_) {
                index -= range_;
            }

            value = data_map_->get(index);
        }

        return value;
    }

public:
    TemporarySkewedArgsGenerator(size_t set_number, size_t range, std::vector<int64_t> hot_times,
                                 std::vector<int64_t> relax_times, std::vector<size_t> set_begins,
                                 std::vector<DistributionPtr> hot_dists, DistributionPtr relax_dist,
                                 DataMapPtr data_map)
        : hot_dists_(std::move(hot_dists)),
          relax_dist_(std::move(relax_dist)),
          data_map_(std::move(data_map)),
          hot_times_(std::move(hot_times)),
          relax_times_(std::move(relax_times)),
          set_begins_(std::move(set_begins)),
          set_number_(set_number),
          range_(range),
          time_(0),
          pointer_(0),
          is_relax_time_(false) {
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
        --time_;
        KeyType left = next();
        KeyType right = next();

        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~TemporarySkewedArgsGenerator() override = default;
};

}  // namespace microbench::workload
