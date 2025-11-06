//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_ZIPFIAN_DISTRIBUTION_BUILDER_H
#define SETBENCH_ZIPFIAN_DISTRIBUTION_BUILDER_H

#include <algorithm>
#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"
#include "workloads/distributions/distribution_builder.h"
#include "workloads/distributions/impls/zipf_distribution.h"

struct ZipfianDistributionBuilder : public MutableDistributionBuilder {
    PAD;
    double alpha = 1;
    PAD;

    ZipfianDistributionBuilder *setAlpha(double _alpha) {
        alpha = _alpha;
        return this;
    }

    ZipfDistribution *build(Random64 &rng, size_t range) override {
        return new ZipfDistribution(rng, alpha, range);
    }

    ZipfDistribution *build(Random64 &rng) override {
        return new ZipfDistribution(rng, alpha);
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "ZipfianDistributionBuilder";
        j["alpha"] = alpha;
    }

    void fromJson(const nlohmann::json &j) override {
        alpha = j["alpha"];
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Zipfian", indents)
        + indented_title_with_data("alpha", alpha, indents);
    };

    ~ZipfianDistributionBuilder() override = default;
};


#endif //SETBENCH_ZIPFIAN_DISTRIBUTION_BUILDER_H
