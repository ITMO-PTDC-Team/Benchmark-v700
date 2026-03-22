//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include <atomic>

#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

using KeyType = int64_t;

/**
    старички + волна
    n — g — gx — gk — cp — ca
        n — количество элементов
        g (grand) — процент старичков
            // 100% - g — процент новичков
        gy — вероятность их вызова
            // 100% - gx — вероятность вызова новичков
        gk — на сколько стары наши старички
            | преподсчёт - перед началом теста делаем gk количество запросов к старичкам
        cp (child) — распределение вызовов среди новичков
            // по умолчанию Zipf 1
            // при желании можно сделать cx — cy
        ca (child add) — вероятность добавления нового элемента
            // 100% - gx - ca — чтение новичка, ca добавление нового новичка
            // при желании можно переработать на "100% - ca — чтение, ca — запись"
                (то есть брать ca не от общей вероятности, а только при чтении"
 */
class CreakersAndWaveArgsGenerator : public ArgsGenerator {
    PAD;
    Random64& rng_;
    PAD;
    double creakers_ratio_;
    size_t creakers_begin_;
    DistributionPtr creakers_dist_;
    MutableDistributionPtr wave_dist_;
    DataMapPtr data_map_;
    PAD;
    std::atomic<size_t> wave_begin_;
    PAD;
    std::atomic<size_t> wave_end_;
    PAD;

    /**
     *
     * @return true - creakers, false - wave
     */
    bool next_coin() {
        double z;  // Uniform random number (0 < z < 1)

        do {
            z = (rng_.next() / (double)rng_.max_value);
        } while ((z == 0) || (z == 1));

        return z < creakers_ratio_;
    }

    KeyType get_creaker() {
        /**
         *                             creakersBegin
         * |________________________________|,,,,,,,,,,,,,,,,,,,,|
         *
         * |,,,,| --- creakers
         * |____| --- wave or unused data
         */

        return data_map_->get(creakers_begin_ + creakers_dist_->next());
    }

    KeyType get_wave() {
        /**
         * In waveDist the first indexes have a higher probability
         */
        size_t local_wave_begin = wave_begin_.load();
        size_t local_wave_end = wave_end_.load();
        size_t local_wave_length;
        size_t index;
        if (local_wave_end < local_wave_begin) {
            /**
             *       waveEnd                waveBegin      creakersBegin
             * |.......|________________________|.............|,,,,,,,,,,,,,,,,,,,,|
             *
             * |....| --- wave
             * |,,,,| --- creakers
             * |____| --- unused data
             */
            local_wave_length = (local_wave_end + creakers_begin_) - local_wave_begin;
            index = local_wave_begin + wave_dist_->next(local_wave_length);

            if (index >= creakers_begin_) {
                index -= creakers_begin_;
            }
        } else {
            local_wave_length = local_wave_end - local_wave_begin;
            index = local_wave_begin + wave_dist_->next(local_wave_length);
        }

        return data_map_->get(index);
    };

    KeyType wave_shift(std::atomic<size_t>& wave_edge) {
        size_t local_wave_edge = wave_edge.load();
        size_t new_wave_edge;
        if (local_wave_edge == 0) {
            new_wave_edge = creakers_begin_ - 1;
        } else {
            new_wave_edge = local_wave_edge - 1;
        }
        wave_edge.compare_exchange_weak(local_wave_edge, new_wave_edge);
        return data_map_->get(new_wave_edge);
    }

public:
    CreakersAndWaveArgsGenerator(Random64& rng, double creakers_ratio, size_t creakers_begin,
                                 size_t wave_begin, size_t wave_end, DistributionPtr creakers_dist,
                                 MutableDistributionPtr wave_dist, DataMapPtr data_map)
        : rng_(rng),
          creakers_ratio_(creakers_ratio),
          creakers_begin_(creakers_begin),
          wave_begin_(wave_begin),
          wave_end_(wave_end),
          creakers_dist_(std::move(creakers_dist)),
          wave_dist_(std::move(wave_dist)),
          data_map_(std::move(data_map)) {
    }

    KeyType next_get() override {
        KeyType value;
        if (next_coin()) {
            value = get_creaker();
        } else {
            value = get_wave();
        }
        return value;
    }

    KeyType next_insert() override {
        return wave_shift(wave_begin_);
    }

    KeyType next_remove() override {
        return wave_shift(wave_end_);
    }

    std::pair<KeyType, KeyType> next_range() override {
        KeyType left;
        KeyType right;
        if (next_coin()) {
            left = get_creaker();
            right = get_creaker();
        } else {
            left = get_wave();
            right = get_wave();
        }
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~CreakersAndWaveArgsGenerator() override = default;
};

}  // namespace microbench::workload
