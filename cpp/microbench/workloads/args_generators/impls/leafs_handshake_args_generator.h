//
// Created by Ravil Galiev on 20.09.2023.
//
#pragma once

#include <atomic>
#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

namespace microbench::workload {

template <typename K>
class LeafsHandshakeArgsGenerator : public ArgsGenerator<K> {
    size_t range_;

    Distribution* read_distribution_;
    MutableDistribution* insert_distribution_;
    Distribution* remove_distribution_;
    Random64& rng_;
    PAD;
    std::atomic<size_t>* deleted_value_;
    PAD;

    DataMap<K>* read_data_;
    DataMap<K>* remove_data_;

public:
    LeafsHandshakeArgsGenerator(Random64& rng, size_t range, std::atomic<size_t>* deleted_value,
                                Distribution* read_distribution,
                                MutableDistribution* insert_distribution,
                                Distribution* remove_distribution, DataMap<K>* read_data,
                                DataMap<K>* remove_data)
        : range_(range),
          read_distribution_(read_distribution),
          insert_distribution_(insert_distribution),
          remove_distribution_(remove_distribution),
          rng_(rng),
          deleted_value_(deleted_value),
          read_data_(read_data),
          remove_data_(remove_data) {
    }

    K next_get() {
        return read_data_->get(read_distribution_->next());
    }

    K next_insert() {
        size_t local_deleted_value = *deleted_value_;

        size_t value;

        bool is_right = rng_.nextDouble() >= 0.5;

        if (local_deleted_value == 1 || (is_right && local_deleted_value != range_)) {
            value =
                local_deleted_value + insert_distribution_->next(range_ - local_deleted_value) + 1;
        } else {
            value = local_deleted_value - insert_distribution_->next(local_deleted_value - 1) - 1;
        }

        return value;
    }

    K next_remove() {
        size_t local_deleted_value = *deleted_value_;
        size_t value = remove_data_->get(remove_distribution_->next());

        // todo learn the difference between all kinds of weakCompareAndSet
        deleted_value_->compare_exchange_weak(local_deleted_value, value);

        return value;
    }

    std::pair<K, K> next_range() {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~LeafsHandshakeArgsGenerator() {
        delete read_data_;
        delete remove_data_;
        delete read_distribution_;
        delete insert_distribution_;
        delete remove_distribution_;
    }
};

}  // namespace microbench::workload

#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

namespace microbench::workload {

class LeafsHandshakeArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range_;

    DistributionBuilder* read_dist_builder_ = new UniformDistributionBuilder();
    MutableDistributionBuilder* insert_dist_builder_ = new ZipfianDistributionBuilder();
    DistributionBuilder* remove_dist_builder_ = new UniformDistributionBuilder();

    DataMapBuilder* read_data_map_builder_ = new IdDataMapBuilder();
    DataMapBuilder* remove_data_map_builder_ = new IdDataMapBuilder();
    std::atomic<size_t>* deleted_value_;

public:
    LeafsHandshakeArgsGeneratorBuilder* set_read_dist_builder(
        DistributionBuilder* read_dist_builder) {
        read_dist_builder_ = read_dist_builder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder* set_insert_dist_builder(
        MutableDistributionBuilder* insert_dist_builder) {
        insert_dist_builder_ = insert_dist_builder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder* set_remove_dist_builder(
        DistributionBuilder* remove_dist_builder) {
        remove_dist_builder_ = remove_dist_builder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder* set_read_data_map_builder(
        DataMapBuilder* read_data_map_builder) {
        read_data_map_builder_ = read_data_map_builder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder* set_remove_data_map_builder(
        DataMapBuilder* remove_data_map_builder) {
        remove_data_map_builder_ = remove_data_map_builder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        //        readDataMapBuilder->init(_range);
        //        removeDataMapBuilder->init(_range);
        deleted_value_ = new std::atomic<size_t>(range_ / 2);

        return this;
    }

    LeafsHandshakeArgsGenerator<K>* build(Random64& rng) override {
        return new LeafsHandshakeArgsGenerator<K>(
            rng, range_, deleted_value_, read_dist_builder_->build(rng, range_),
            insert_dist_builder_->build(rng), remove_dist_builder_->build(rng, range_),
            read_data_map_builder_->build(), remove_data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "LeafsHandshakeArgsGeneratorBuilder";
        j["readDistBuilder"] = *read_dist_builder_;
        j["insertDistBuilder"] = *insert_dist_builder_;
        j["removeDistBuilder"] = *remove_dist_builder_;
        j["readDataMapBuilder"] = *read_data_map_builder_;
        j["removeDataMapBuilder"] = *remove_data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        read_dist_builder_ = get_distribution_from_json(j["readDistBuilder"]);
        insert_dist_builder_ = get_mutable_distribution_from_json(j["insertDistBuilder"]);
        remove_dist_builder_ = get_distribution_from_json(j["removeDistBuilder"]);
        read_data_map_builder_ = get_data_map_from_json(j["readDataMapBuilder"]);
        remove_data_map_builder_ = get_data_map_from_json(j["removeDataMapBuilder"]);
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "LEAFS_HANDSHAKE", indents) +
               indented_title("Read Distribution", indents) +
               read_dist_builder_->to_string(indents + 1) +
               indented_title("Insert Distribution", indents) +
               insert_dist_builder_->to_string(indents + 1) +
               indented_title("Remove Distribution", indents) +
               remove_dist_builder_->to_string(indents + 1) +
               indented_title("Read Data Map", indents) +
               read_data_map_builder_->to_string(indents + 1) +
               indented_title("Remove Data Map", indents) +
               remove_data_map_builder_->to_string(indents + 1);
    }

    ~LeafsHandshakeArgsGeneratorBuilder() override {
        delete read_dist_builder_;
        delete insert_dist_builder_;
        delete remove_dist_builder_;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};

}  // namespace microbench::workload
