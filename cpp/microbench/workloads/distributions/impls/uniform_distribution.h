//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

class UniformDistribution : public MutableDistribution {
private:
    PAD;
    Random64& rng_;
    size_t range_;
    PAD;

public:
    explicit UniformDistribution(Random64& rng, const size_t range = 0)
        : rng_(rng),
          range_(range) {
    }

    void set_range(size_t max_key) override {
        range_ = max_key;
    }

    size_t next() override {
        size_t result = rng_.next(range_);
        return result;
    }

    ~UniformDistribution() override = default;
};

}  // namespace microbench::workload
