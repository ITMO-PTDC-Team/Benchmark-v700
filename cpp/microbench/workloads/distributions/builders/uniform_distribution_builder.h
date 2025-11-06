//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_UNIFORM_DISTRIBUTION_BUILDER_H
#define SETBENCH_UNIFORM_DISTRIBUTION_BUILDER_H

#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution_builder.h"
#include "workloads/distributions/impls/uniform_distribution.h"
#include "globals_extern.h"

struct UniformDistributionBuilder : public MutableDistributionBuilder {
    UniformDistribution * build(Random64 &rng, size_t range) override {
        return new UniformDistribution(rng, range);
    }

    UniformDistribution * build(Random64 &rng) override {
        return new UniformDistribution(rng);
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "UniformDistributionBuilder";
    }

    void fromJson(const nlohmann::json &j) override {

    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Uniform", indents);
    }

    ~UniformDistributionBuilder() override = default;
};

#endif //SETBENCH_UNIFORM_DISTRIBUTION_BUILDER_H
