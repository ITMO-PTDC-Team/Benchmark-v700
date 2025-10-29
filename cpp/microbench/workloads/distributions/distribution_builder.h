//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <string>
#include "distribution.h"
#include "random_xoshiro256p.h"
#include "json/single_include/nlohmann/json.hpp"

struct DistributionBuilder {
    virtual Distribution* build(Random64& rng, size_t range) = 0;

    virtual std::string to_string(size_t indents) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual ~DistributionBuilder() = default;
};

struct MutableDistributionBuilder : public DistributionBuilder {
    virtual MutableDistribution* build(Random64& rng) = 0;
};

void to_json(nlohmann::json& j, const DistributionBuilder& s) {
    s.to_json(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json& j, DistributionBuilder& s) {
    s.from_json(j);
}
