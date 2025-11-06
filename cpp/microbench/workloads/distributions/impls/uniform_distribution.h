//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_UNIFORM_DISTRIBUTION_H
#define SETBENCH_UNIFORM_DISTRIBUTION_H

#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

class UniformDistribution : public MutableDistribution {
private:
    PAD;
    Random64 &rng;
    size_t range;
    PAD;
public:
    UniformDistribution(Random64 &_rng, const size_t _range = 0) : rng(_rng), range(_range) {}

    void setRange(size_t _maxKey) override {
        range = _maxKey;
    }

    size_t next() override {
        size_t result = rng.next(range);
        return result;
    }

    ~UniformDistribution() override = default;
};

#endif //SETBENCH_UNIFORM_DISTRIBUTION_H
