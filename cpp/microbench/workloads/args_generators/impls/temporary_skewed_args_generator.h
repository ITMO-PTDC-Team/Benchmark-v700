//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

#include "globals_extern.h"

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
template <typename K>
class TemporarySkewedArgsGenerator : public ArgsGenerator<K> {
    size_t time_;
    size_t pointer_;
    bool is_relax_time_;

    Distribution** hot_dists_;
    Distribution* relax_dist_;
    DataMap<K>* data_map_;
    // PAD;
    int64_t* hot_times_;
    // PAD;
    int64_t* relax_times_;
    // PAD;
    size_t* set_begins_;
    // PAD;
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

    K next() {
        update_pointer();
        K value;

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
    TemporarySkewedArgsGenerator(size_t set_number, size_t range, int64_t* hot_times,
                                 int64_t* relax_times, size_t* set_begins, Distribution** hot_dists,
                                 Distribution* relax_dist, DataMap<K>* data_map)
        : hot_dists_(hot_dists),
          relax_dist_(relax_dist),
          data_map_(data_map),
          hot_times_(hot_times),
          relax_times_(relax_times),
          set_begins_(set_begins),
          set_number_(set_number),
          range_(range),
          time_(0),
          pointer_(0),
          is_relax_time_(false) {
    }

    K next_get() override {
        return next();
    }

    K next_insert() override {
        return next();
    }

    K next_remove() override {
        return next();
    }

    std::pair<K, K> next_range() override {
        --time_;
        K left = next();
        K right = next();

        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~TemporarySkewedArgsGenerator() override {
        delete[] hot_dists_;
        delete relax_dist_;
        //        delete dataMap; //TODO may deleted twice
    };
};

}  // namespace microbench::workload
