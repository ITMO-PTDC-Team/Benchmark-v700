//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_DISTRIBUTION_BUILDER_H
#define SETBENCH_DISTRIBUTION_BUILDER_H

#include <string>
#include "distribution.h"
#include "random_xoshiro256p.h"
#include <nlohmann/json.hpp>

struct DistributionBuilder {
    virtual std::shared_ptr<Distribution> build(Random64 &rng, size_t range) = 0;

    virtual std::string toString(size_t indents) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual ~DistributionBuilder() = default;
};

struct MutableDistributionBuilder : public DistributionBuilder {
    virtual std::shared_ptr<MutableDistribution> build(Random64 &rng) = 0;
};

void to_json(nlohmann::json &j, const DistributionBuilder &s) {
    s.toJson(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json &j, DistributionBuilder &s) {
    s.fromJson(j);
}

#endif //SETBENCH_DISTRIBUTION_BUILDER_H
