#pragma once

#include <memory>
#include "args_generators/args_generator.h"
#include "distributions/builders/zipfian_distribution_builder.h"
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

    DistributionBuilderPtr read_dist_builder_ = std::make_unique<UniformDistributionBuilder>();
    MutableDistributionBuilderPtr insert_dist_builder_ =
        std::make_unique<ZipfianDistributionBuilder>();
    DistributionBuilderPtr remove_dist_builder_ = std::make_unique<UniformDistributionBuilder>();

    DataMapBuilderPtr read_data_map_builder_ = std::make_unique<IdDataMapBuilder>();
    DataMapBuilderPtr remove_data_map_builder_ = std::make_unique<IdDataMapBuilder>();
    std::atomic<size_t> deleted_value_;

public:
    LeafsHandshakeArgsGeneratorBuilder& set_read_dist_builder(
        DistributionBuilderPtr read_dist_builder) {
        read_dist_builder_ = std::move(read_dist_builder);
        return *this;
    }

    LeafsHandshakeArgsGeneratorBuilder& set_insert_dist_builder(
        MutableDistributionBuilderPtr insert_dist_builder) {
        insert_dist_builder_ = std::move(insert_dist_builder);
        return *this;
    }

    LeafsHandshakeArgsGeneratorBuilder& set_remove_dist_builder(
        DistributionBuilderPtr remove_dist_builder) {
        remove_dist_builder_ = std::move(remove_dist_builder);
        return *this;
    }

    LeafsHandshakeArgsGeneratorBuilder& set_read_data_map_builder(
        DataMapBuilderPtr read_data_map_builder) {
        read_data_map_builder_ = std::move(read_data_map_builder);
        return *this;
    }

    LeafsHandshakeArgsGeneratorBuilder& set_remove_data_map_builder(
        DataMapBuilderPtr remove_data_map_builder) {
        remove_data_map_builder_ = std::move(remove_data_map_builder);
        return *this;
    }

    LeafsHandshakeArgsGeneratorBuilder& init(size_t range) override {
        range_ = range;
        deleted_value_ = range_ / 2;

        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_unique<LeafsHandshakeArgsGenerator>(
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
        read_data_map_builder_ = (get_data_map_from_json(j["readDataMapBuilder"]));
        remove_data_map_builder_ = (get_data_map_from_json(j["removeDataMapBuilder"]));
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

    ~LeafsHandshakeArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
