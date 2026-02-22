#pragma once

#include "workloads/args_generators/impls/range_query_args_generator.h"
#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

namespace microbench::workload {

class RangeQueryArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range_;
    size_t interval_;

public:
    DistributionBuilder* distributionBuilder = new UniformDistributionBuilder();
    DataMapBuilder* dataMapBuilder = new IdDataMapBuilder();

    RangeQueryArgsGeneratorBuilder* set_distribution_builder(
        DistributionBuilder* distribution_builder) {
        distributionBuilder = distribution_builder;
        return this;
    }

    RangeQueryArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        dataMapBuilder = data_map_builder;
        return this;
    }

    RangeQueryArgsGeneratorBuilder* set_interval(size_t interval) {
        interval_ = interval;
        return this;
    }

    RangeQueryArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        return this;
    }

    RangeQueryArgsGenerator<K>* build(Random64& rng) override {
        return new RangeQueryArgsGenerator<K>(dataMapBuilder->build(),
                                              distributionBuilder->build(rng, range_), interval_);
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "RangeQueryArgsGeneratorBuilder";
        j["interval"] = interval_;
        j["distributionBuilder"] = *distributionBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void from_json(const nlohmann::json& j) override {
        interval_ = j["interval"];
        distributionBuilder = get_distribution_from_json(j["distributionBuilder"]);
        dataMapBuilder = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "RangeQuery", indents);
        res += indented_title_with_str_data("Interval", std::to_string(interval_), indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->to_string(indents + 1);
        res += indented_title("Data Map", indents);
        res += dataMapBuilder->to_string(indents + 1);
        return res;
    }

    ~RangeQueryArgsGeneratorBuilder() override {
        delete distributionBuilder;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};

}  // namespace microbench::workload
