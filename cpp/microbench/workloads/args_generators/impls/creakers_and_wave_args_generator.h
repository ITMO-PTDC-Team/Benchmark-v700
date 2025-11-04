//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include <atomic>

#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

#include "globals_extern.h"

namespace microbench::workload {

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
template <typename K>
class CreakersAndWaveArgsGenerator : public ArgsGenerator<K> {
    PAD;
    Random64& rng_;
    PAD;
    double creakers_ratio_;
    size_t creakers_begin_;
    Distribution* creakers_dist_;
    MutableDistribution* wave_dist_;
    DataMap<K>* data_map_;
    PAD;
    std::atomic<size_t>* wave_begin_;
    PAD;
    std::atomic<size_t>* wave_end_;
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

    K get_creaker() {
        /**
         *                             creakersBegin
         * |________________________________|,,,,,,,,,,,,,,,,,,,,|
         *
         * |,,,,| --- creakers
         * |____| --- wave or unused data
         */

        return data_map_->get(creakers_begin_ + creakers_dist_->next());
    }

    K get_wave() {
        /**
         * In waveDist the first indexes have a higher probability
         */
        size_t local_wave_begin = *wave_begin_;
        size_t local_wave_end = *wave_end_;
        //            size_t localWaveLength = (localWaveEnd - localWaveBegin + creakersBegin) %
        //            creakersBegin;
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

    K wave_shift(std::atomic<size_t>* wave_edge) {
        size_t local_wave_edge = *wave_edge;
        size_t new_wave_edge;
        if (local_wave_edge == 0) {
            new_wave_edge = creakers_begin_ - 1;
        } else {
            new_wave_edge = local_wave_edge - 1;
        }
        wave_edge->compare_exchange_weak(local_wave_edge, new_wave_edge);
        return data_map_->get(new_wave_edge);
    }

public:
    CreakersAndWaveArgsGenerator(Random64& rng, double creakers_ratio, size_t creakers_begin,
                                 std::atomic<size_t>* wave_begin, std::atomic<size_t>* wave_end,
                                 Distribution* creakers_dist, MutableDistribution* wave_dist,
                                 DataMap<K>* data_map)
        : rng_(rng),
          creakers_ratio_(creakers_ratio),
          creakers_begin_(creakers_begin),
          wave_begin_(wave_begin),
          wave_end_(wave_end),
          creakers_dist_(creakers_dist),
          wave_dist_(wave_dist),
          data_map_(data_map) {
    }

    K next_get() override {
        K value;
        if (next_coin()) {
            value = get_creaker();
        } else {
            value = get_wave();
        }
        return value;
    }

    K next_insert() override {
        return wave_shift(wave_begin_);
    }

    K next_remove() override {
        return wave_shift(wave_end_);
    }

    std::pair<K, K> next_range() override {
        K left;
        K right;
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

    ~CreakersAndWaveArgsGenerator() override {
        delete creakers_dist_;
        delete wave_dist_;
        delete data_map_;
    };
};

}  // namespace microbench::workload

#include "errors.h"

namespace microbench::workload {
template <typename K>
class CreakersAndWavePrefillArgsGenerator : public ArgsGenerator<K> {
    PAD;
    Random64& rng_;
    PAD;
    DataMap<K>* data_map_;
    size_t wave_begin_;
    size_t prefill_length_;
    PAD;

public:
    CreakersAndWavePrefillArgsGenerator(Random64& rng, size_t wave_begin, size_t prefill_length,
                                        DataMap<K>* data_map)
        : rng_(rng),
          wave_begin_(wave_begin),
          prefill_length_(prefill_length),
          data_map_(data_map) {
    }

    K next_get() override{setbench_error("Unsupported operation -- nextGet")}

    K next_insert() override {
        /**
         *                       waveBegin           creakersBegin
         * |_________________________|....................|,,,,,,,,,,,,,,,,,,,,|
         *                           |<--          prefillLength            -->|
         * |....| --- wave
         * |,,,,| --- creakers
         * |____| --- unused data
         */
        return data_map_->get(wave_begin_ + rng_.next(prefill_length_));
    }

    K next_remove() override{setbench_error("Unsupported operation -- nextRemove")}

    std::pair<K, K> next_range() override {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~CreakersAndWavePrefillArgsGenerator() override {
        delete data_map_;
    };
};

}  // namespace microbench::workload

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"

namespace microbench::workload {

class CreakersAndWaveArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t creakers_length_;
    size_t creakers_begin_;
    size_t start_wave_length_;
    PAD;
    std::atomic<size_t>* wave_begin_;
    PAD;
    std::atomic<size_t>* wave_end_;
    PAD;

    double creakers_size_ = 0;
    double creakers_ratio_ = 0;
    double wave_size_ = 0;

    DistributionBuilder* creakers_dist_builder_ = new UniformDistributionBuilder();
    MutableDistributionBuilder* wave_dist_builder_ = new ZipfianDistributionBuilder();

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

public:
    double get_creakers_size() const {
        return creakers_size_;
    }

    double get_wave_size() const {
        return wave_size_;
    }

    DataMapBuilder* get_data_map_builder() const {
        return data_map_builder_;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_creakers_size(double creakers_size) {
        creakers_size_ = creakers_size;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_creakers_ratio(double creakers_ratio) {
        creakers_ratio_ = creakers_ratio;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_wave_size(double wave_size) {
        wave_size_ = wave_size;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_creakers_dist_builder(
        DistributionBuilder* creakers_dist_builder) {
        creakers_dist_builder_ = creakers_dist_builder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_wave_dist_builder(
        MutableDistributionBuilder* wave_dist_builder) {
        wave_dist_builder_ = wave_dist_builder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder* init(size_t range) override {
        //        dataMapBuilder->init(range);
        creakers_length_ = range * creakers_size_;
        creakers_begin_ = range - creakers_length_;
        start_wave_length_ = range * wave_size_;
        wave_end_ = new std::atomic<size_t>(creakers_begin_);
        wave_begin_ = new std::atomic<size_t>(*wave_end_ - start_wave_length_);
        return this;
    }

    CreakersAndWaveArgsGenerator<K>* build(Random64& rng) override {
        return new CreakersAndWaveArgsGenerator<K>(
            rng, creakers_ratio_, creakers_begin_, wave_begin_, wave_end_,
            creakers_dist_builder_->build(rng, creakers_length_), wave_dist_builder_->build(rng),
            data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "CreakersAndWaveArgsGeneratorBuilder";
        j["creakersRatio"] = creakers_ratio_;
        j["creakersSize"] = creakers_size_;
        j["waveSize"] = wave_size_;
        j["creakersDistBuilder"] = *creakers_dist_builder_;
        j["waveDistBuilder"] = *wave_dist_builder_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        creakers_ratio_ = j["creakersRatio"];
        creakers_size_ = j["creakersSize"];
        wave_size_ = j["waveSize"];
        creakers_dist_builder_ = get_distribution_from_json(j["creakersDistBuilder"]);
        wave_dist_builder_ = get_mutable_distribution_from_json(j["waveDistBuilder"]);
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE", indents) +
               indented_title_with_data("Creakers Ratio", creakers_ratio_, indents) +
               indented_title_with_data("Creakers Size", creakers_size_, indents) +
               indented_title_with_data("Wave Size", wave_size_, indents) +
               indented_title("Creakers Distribution", indents) +
               creakers_dist_builder_->to_string(indents + 1) +
               indented_title("Wave Distribution", indents) +
               wave_dist_builder_->to_string(indents + 1) + indented_title("Data Map", indents) +
               data_map_builder_->to_string(indents + 1);
    }

    ~CreakersAndWaveArgsGeneratorBuilder() override {
        delete wave_begin_;
        delete wave_end_;
        delete creakers_dist_builder_;
        delete wave_dist_builder_;
        //        delete dataMapBuilder;
    };
};

class CreakersAndWavePrefillArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t wave_begin_;
    size_t prefill_length_;
    PAD;

    double creakers_size_ = 0;
    double wave_size_ = 0;

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

public:
    size_t get_prefill_length() const {
        return prefill_length_;
    }

    size_t get_prefill_length(size_t range) const {
        return range * creakers_size_ + range * wave_size_;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder() {
    }

    explicit CreakersAndWavePrefillArgsGeneratorBuilder(
        CreakersAndWaveArgsGeneratorBuilder* builder) {
        set_parameters_by_builder(builder);
    }

    CreakersAndWavePrefillArgsGeneratorBuilder* set_parameters_by_builder(
        CreakersAndWaveArgsGeneratorBuilder* builder) {
        creakers_size_ = builder->get_creakers_size();
        wave_size_ = builder->get_wave_size();
        data_map_builder_ = builder->get_data_map_builder();
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder* set_creakers_size(double creakers_size) {
        creakers_size_ = creakers_size;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder* set_wave_size(double wave_size) {
        wave_size_ = wave_size;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder* set_data_map_builder(
        DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder* init(size_t range) override {
        //        dataMapBuilder->init(range);
        prefill_length_ = range * creakers_size_ + range * wave_size_;
        wave_begin_ = range - prefill_length_;
        return this;
    }

    CreakersAndWavePrefillArgsGenerator<K>* build(Random64& rng) override {
        return new CreakersAndWavePrefillArgsGenerator<K>(rng, wave_begin_, prefill_length_,
                                                          data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "CreakersAndWavePrefillArgsGeneratorBuilder";
        j["creakersSize"] = creakers_size_;
        j["waveSize"] = wave_size_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        creakers_size_ = j["creakersSize"];
        wave_size_ = j["waveSize"];
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE_PREFILL", indents) +
               indented_title_with_data("Creakers Size", creakers_size_, indents) +
               indented_title_with_data("Wave Size", wave_size_, indents) +
               indented_title("Data Map", indents) + data_map_builder_->to_string(indents + 1);
    }

    ~CreakersAndWavePrefillArgsGeneratorBuilder() override {
        //        delete dataMapBuilder;
    };
};

}  // namespace microbench::workload
