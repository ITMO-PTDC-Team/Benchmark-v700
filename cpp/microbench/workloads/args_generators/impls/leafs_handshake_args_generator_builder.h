#pragma once

#include "workloads/args_generators/impls/leafs_handshake_args_generator.h"
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
