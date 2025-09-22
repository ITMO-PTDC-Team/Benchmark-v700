//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_BUILDER_H
#define SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_BUILDER_H

#include <algorithm>
#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"
#include "workloads/distributions/distribution_builder.h"
#include "uniform_distribution_builder.h"
#include "workloads/distributions/impls/skewed_uniform_distribution.h"

std::shared_ptr<DistributionBuilder> getDistributionFromJson(const nlohmann::json &j);

struct SkewedUniformDistributionBuilder : public DistributionBuilder {
    PAD;
    double hotSize = 0;
    double hotRatio = 0;

    std::shared_ptr<DistributionBuilder> hotDistBuilder = std::make_shared<UniformDistributionBuilder>();
    std::shared_ptr<DistributionBuilder> coldDistBuilder = std::make_shared<UniformDistributionBuilder>();
    PAD;

    SkewedUniformDistributionBuilder *setHotSize(double _hotSize) {
        hotSize = _hotSize;
        return this;
    }

    SkewedUniformDistributionBuilder *setHotRatio(double _hotRatio) {
        hotRatio = _hotRatio;
        return this;
    }

    SkewedUniformDistributionBuilder *setHotDistBuilder(std::shared_ptr<DistributionBuilder> _hotDistBuilder) {
        hotDistBuilder = _hotDistBuilder;
        return this;
    }

    SkewedUniformDistributionBuilder *setColdDistBuilder(std::shared_ptr<DistributionBuilder> _coldDistBuilder) {
        coldDistBuilder = _coldDistBuilder;
        return this;
    }

    size_t getHotLength(size_t range) const {
        return (size_t) (range * hotSize);
    }

    size_t getColdLength(size_t range) const {
        return range - getHotLength(range);
    }

    std::shared_ptr<Distribution> build(Random64 &rng, size_t range) override {
        return std::shared_ptr<SkewedUniformDistribution>(new SkewedUniformDistribution(
                rng,
                hotDistBuilder->build(rng, getHotLength(range)),
                coldDistBuilder->build(rng, getColdLength(range)),
                hotRatio, getHotLength(range)
        ));
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "SkewedUniformDistributionBuilder";
        j["hotSize"] = hotSize;
        j["hotRatio"] = hotRatio;
        j["hotDistBuilder"] = *hotDistBuilder;
        j["coldDistBuilder"] = *coldDistBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        hotSize = j["hotSize"];
        hotRatio = j["hotRatio"];

        hotDistBuilder = getDistributionFromJson(j["hotDistBuilder"]);
        coldDistBuilder = getDistributionFromJson(j["coldDistBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Skewed Uniform", indents)
               + indented_title_with_data("HOT SIZE", hotSize, indents)
               + indented_title_with_data("HOT RATIO", hotRatio, indents);
    }

    ~SkewedUniformDistributionBuilder() override = default;
};


#endif //SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_BUILDER_H
