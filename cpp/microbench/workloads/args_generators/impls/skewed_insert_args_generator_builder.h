#pragma once

#include <memory>
#include "args_generators/args_generator.h"
#include "data_maps/data_map_json_convector.h"
#include "distributions/distribution_json_convector.h"
#include "workloads/args_generators/impls/skewed_insert_args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"

namespace microbench::workload {

class SkewedInsertArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range_;
    DistributionBuilderPtr dist_builder_ = std::make_shared<UniformDistributionBuilder>();
    DataMapBuilderPtr data_map_builder_ = std::make_shared<ArrayDataMapBuilder>();
    double skewed_size_ = 0;
    size_t skewed_length_ = 0;

public:
    SkewedInsertArgsGeneratorBuilder& set_skewed_size(double skewed_size) {
        skewed_size_ = skewed_size;
        return *this;
    }

    SkewedInsertArgsGeneratorBuilder& set_distribution_builder(DistributionBuilderPtr dist_builder) {
        dist_builder_ = std::move(dist_builder);
        return *this;
    }

    SkewedInsertArgsGeneratorBuilder& set_data_map_builder(DataMapBuilderPtr data_map_builder) {
        data_map_builder_ = std::move(data_map_builder);
        return *this;
    }

    SkewedInsertArgsGeneratorBuilder& init(size_t range) override {
        range_ = range;
        skewed_length_ = (size_t)(range * skewed_size_);
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_shared<SkewedInsertArgsGenerator>(
            skewed_length_, dist_builder_->build(rng, range_ - skewed_length_),
            data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "SkewedInsertArgsGeneratorBuilder";
        j["distributionBuilder"] = *dist_builder_;
        j["skewedSize"] = skewed_size_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        dist_builder_ = get_distribution_from_json(j["distributionBuilder"]);
        skewed_size_ = j["skewedSize"];
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_INSERT", indents) +
               indented_title_with_data("Skewed size", skewed_size_, indents) +
               indented_title("Distribution", indents) + dist_builder_->to_string(indents + 1) +
               indented_title("Data Map", indents) + data_map_builder_->to_string(indents + 1);
    }

    ~SkewedInsertArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
