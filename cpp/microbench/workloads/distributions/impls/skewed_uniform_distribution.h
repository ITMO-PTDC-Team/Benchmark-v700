//
// Created by Ravil Galiev on 30.08.2022.
//
#pragma once

#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

class SkewedUniformDistribution : public Distribution {
private:
    PAD;
    Random64& rng_;
    Distribution* hot_distribution_;
    Distribution* cold_distribution_;
    double hot_prob_;
    size_t hot_set_length_;
    PAD;

public:
    SkewedUniformDistribution(Random64& rng, Distribution* hot_distribution,
                              Distribution* cold_distribution, const double hot_prob,
                              const size_t hot_set_length)
        : hot_distribution_(hot_distribution),
          cold_distribution_(cold_distribution),
          rng_(rng),
          hot_prob_(hot_prob),
          hot_set_length_(hot_set_length) {
    }

    size_t next() override {
        size_t value;
        double z;  // Uniform random number (0 < z < 1)
        // Pull a uniform random number (0 < z < 1)
        do {
            z = ((double)rng_.next() / (double)rng_.max_value);
        } while ((z == 0) || (z == 1));
        if (z < hot_prob_) {
            value = hot_distribution_->next();
        } else {
            value = hot_set_length_ + cold_distribution_->next();
        }
        return value;
    }

    ~SkewedUniformDistribution() override {
        delete hot_distribution_;
        delete cold_distribution_;
    }
};
