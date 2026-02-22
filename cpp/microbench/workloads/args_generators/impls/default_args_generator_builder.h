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
    DistributionBuilder* distributionBuilder = new UniformDistributionBuilder();
    DataMapBuilder* dataMapBuilder = new IdDataMapBuilder();

    DefaultArgsGeneratorBuilder* set_distribution_builder(
        DistributionBuilder* distribution_builder) {
        distributionBuilder = distribution_builder;
        return this;
    }

    DefaultArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        dataMapBuilder = data_map_builder;
        return this;
    }

    DefaultArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        //        dataMapBuilder->init(_range);
        return this;
    }

    DefaultArgsGenerator<K>* build(Random64& rng) override {
        return new DefaultArgsGenerator<K>(dataMapBuilder->build(),
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

        //        return indented_title_with_str_data("Type", "DEFAULT", indents)
        //               + indented_title("Distribution", indents)
        //               + distributionBuilder->toString(indents + 1)
        //               + indented_title("Data Map", indents)
        //               + dataMapBuilder->toString(indents + 1);
    }

    ~DefaultArgsGeneratorBuilder() override {
        delete distributionBuilder;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};

}  // namespace microbench::workload
