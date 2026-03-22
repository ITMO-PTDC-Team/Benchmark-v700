//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include <cassert>
#include "distributions/distribution.h"
#include "globals_extern.h"
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution_builder.h"
#include "workloads/distributions/impls/zipf_distribution.h"

namespace microbench::workload {

struct ZipfianDistributionBuilder : public MutableDistributionBuilder {
    PAD;
    double alpha = 1;
    PAD;

    ZipfianDistributionBuilder& set_alpha(double alpha) {
        alpha = alpha;
        return *this;
    }

    DistributionPtr build(Random64& rng, size_t range) override {
        return std::make_shared<ZipfDistribution>(rng, alpha, range);
    }

    MutableDistributionPtr build(Random64& rng) override {
        return std::make_shared<ZipfDistribution>(rng, alpha);
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "ZipfianDistributionBuilder";
        j["alpha"] = alpha;
    }

    void from_json(const nlohmann::json& j) override {
        alpha = j["alpha"];
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Zipfian", indents) +
               indented_title_with_data("alpha", alpha, indents);
    };

    ~ZipfianDistributionBuilder() override = default;
};

}  // namespace microbench::workload
