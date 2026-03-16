#pragma once

#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

namespace microbench::workload {

class DefaultArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range_;

public:
    DistributionBuilderPtr distributionBuilder = std::make_unique<UniformDistributionBuilder>();
    DataMapBuilderPtr dataMapBuilder = std::make_shared<IdDataMapBuilder>();

    DefaultArgsGeneratorBuilder& set_distribution_builder(
        DistributionBuilderPtr distribution_builder) {
        distributionBuilder = std::move(distribution_builder);
        return *this;
    }

    DefaultArgsGeneratorBuilder& set_data_map_builder(DataMapBuilderPtr data_map_builder) {
        dataMapBuilder = data_map_builder;
        return *this;
    }

    DefaultArgsGeneratorBuilder& init(size_t range) override {
        range_ = range;
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_unique<DefaultArgsGenerator>(dataMapBuilder->build(),
                                                      distributionBuilder->build(rng, range_));
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "DefaultArgsGeneratorBuilder";
        j["distributionBuilder"] = *distributionBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void from_json(const nlohmann::json& j) override {
        distributionBuilder = get_distribution_from_json(j["distributionBuilder"]);
        dataMapBuilder = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "DEFAULT", indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->to_string(indents + 1);
        res += indented_title("Data Map", indents);
        res += dataMapBuilder->to_string(indents + 1);
        return res;
    }

    ~DefaultArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
