//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include <cassert>
#include <cmath>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

class ZipfDistribution : public MutableDistribution {
private:
    PAD;
    Random64& rng_;
    size_t last_range_;
    double area_;
    double alpha_;
    PAD;

public:
    explicit ZipfDistribution(Random64& rng, double alpha = 1.0, size_t range = 0)
        : rng_(rng),
          alpha_(alpha) {
        set_range(range);
    }

    void set_range(size_t range) override {
        if (last_range_ == range) {
            return;
        }

        last_range_ = range;

        ++range;
        if (alpha_ == 1.0) {
            area_ = log(range);
        } else {
            area_ = (pow((double)range, 1.0 - alpha_) - 1.0) / (1.0 - alpha_);
        }
    }

    size_t next() override {
        double z;  // Uniform random number (0 < z < 1)
        do {
            z = (rng_.next() / (double)rng_.max_value);
        } while ((z == 0) || (z == 1));
        size_t zipf_value = 0;
        double s = area_ * z;
        if (alpha_ == 1.0) {
            zipf_value = (size_t)exp(s);
        } else {
            zipf_value = (size_t)pow(s * (1.0 - alpha_) + 1.0, 1.0 / (1.0 - alpha_));
        }
        return zipf_value - 1;
    }

    ~ZipfDistribution() override = default;
};

}  // namespace microbench::workload
