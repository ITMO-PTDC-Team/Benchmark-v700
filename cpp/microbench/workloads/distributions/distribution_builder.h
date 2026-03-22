//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <memory>
#include <string>
#include "distribution.h"
#include "random_xoshiro256p.h"
#include "nlohmann/json.hpp"

namespace microbench::workload {

struct DistributionBuilder {
    virtual DistributionPtr build(Random64& rng, size_t range) = 0;

    virtual std::string to_string(size_t indents) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual ~DistributionBuilder() = default;
};

using DistributionBuilderPtr = std::shared_ptr<DistributionBuilder>;

struct MutableDistributionBuilder : public DistributionBuilder {
    virtual MutableDistributionPtr build(Random64& rng) = 0;
};

using MutableDistributionBuilderPtr = std::shared_ptr<MutableDistributionBuilder>;

void to_json(nlohmann::json& j, const DistributionBuilder& s) {
    s.to_json(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json& j, DistributionBuilder& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
