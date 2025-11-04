//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution_builder.h"
#include "uniform_distribution_builder.h"
#include "workloads/distributions/impls/skewed_uniform_distribution.h"

namespace microbench::workload {

DistributionBuilder* get_distribution_from_json(const nlohmann::json& j);

struct SkewedUniformDistributionBuilder : public DistributionBuilder {
    PAD;
    double hotSize = 0;
    double hotRatio = 0;

    DistributionBuilder* hotDistBuilder = new UniformDistributionBuilder();
    DistributionBuilder* coldDistBuilder = new UniformDistributionBuilder();
    PAD;

    SkewedUniformDistributionBuilder* set_hot_size(double hot_size) {
        hotSize = hot_size;
        return this;
    }

    SkewedUniformDistributionBuilder* set_hot_ratio(double hot_ratio) {
        hotRatio = hot_ratio;
        return this;
    }

    SkewedUniformDistributionBuilder* set_hot_dist_builder(DistributionBuilder* hot_dist_builder) {
        hotDistBuilder = hot_dist_builder;
        return this;
    }

    SkewedUniformDistributionBuilder* set_cold_dist_builder(
        DistributionBuilder* cold_dist_builder) {
        coldDistBuilder = cold_dist_builder;
        return this;
    }

    size_t get_hot_length(size_t range) const {
        return (size_t)(range * hotSize);
    }

    size_t get_cold_length(size_t range) const {
        return range - get_hot_length(range);
    }

    SkewedUniformDistribution* build(Random64& rng, size_t range) override {
        return new SkewedUniformDistribution(rng, hotDistBuilder->build(rng, get_hot_length(range)),
                                             coldDistBuilder->build(rng, get_cold_length(range)),
                                             hotRatio, get_hot_length(range));
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "SkewedUniformDistributionBuilder";
        j["hotSize"] = hotSize;
        j["hotRatio"] = hotRatio;
        j["hotDistBuilder"] = *hotDistBuilder;
        j["coldDistBuilder"] = *coldDistBuilder;
    }

    void from_json(const nlohmann::json& j) override {
        hotSize = j["hotSize"];
        hotRatio = j["hotRatio"];

        hotDistBuilder = get_distribution_from_json(j["hotDistBuilder"]);
        coldDistBuilder = get_distribution_from_json(j["coldDistBuilder"]);
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Skewed Uniform", indents) +
               indented_title_with_data("HOT SIZE", hotSize, indents) +
               indented_title_with_data("HOT RATIO", hotRatio, indents);
    }

    ~SkewedUniformDistributionBuilder() override {
        delete hotDistBuilder;
        delete coldDistBuilder;
    }
};

}  // namespace microbench::workload
