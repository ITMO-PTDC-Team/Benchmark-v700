#pragma once

#include "workloads/args_generators/impls/skewed_insert_args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"

namespace microbench::workload {

class SkewedInsertArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range_;

    DistributionBuilder* dist_builder_ = new UniformDistributionBuilder();

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

    double skewed_size_ = 0;

    size_t skewed_length_;

public:
    SkewedInsertArgsGeneratorBuilder* set_skewed_size(double skewed_size) {
        skewed_size_ = skewed_size;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* set_distribution_builder(DistributionBuilder* dist_builder) {
        dist_builder_ = dist_builder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        //        dataMapBuilder->init(range);
        skewed_length_ = (size_t)(range * skewed_size_);
        return this;
    }

    SkewedInsertArgsGenerator<K>* build(Random64& rng) override {
        return new SkewedInsertArgsGenerator<K>(skewed_length_,
                                                dist_builder_->build(rng, range_ - skewed_length_),
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

    ~SkewedInsertArgsGeneratorBuilder() override {
        delete dist_builder_;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};

}  // namespace microbench::workload
