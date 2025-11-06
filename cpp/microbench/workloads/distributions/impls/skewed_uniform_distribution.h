//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_H
#define SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_H

#include <algorithm>
#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

class SkewedUniformDistribution : public Distribution {
private:
    PAD;
    Random64 &rng;
    Distribution *hotDistribution;
    Distribution *coldDistribution;
    double hotProb;
    size_t hotSetLength;
    PAD;
public:
    SkewedUniformDistribution(Random64 &_rng,
                              Distribution *_hotDistribution, Distribution *_coldDistribution,
                              const double _hotProb, const size_t _hotSetLength)
            : hotDistribution(_hotDistribution), coldDistribution(_coldDistribution),
              rng(_rng), hotProb(_hotProb), hotSetLength(_hotSetLength) {}


    size_t next() override {
        size_t value;
        double z; // Uniform random number (0 < z < 1)
        // Pull a uniform random number (0 < z < 1)
        do {
            z = ((double) rng.next() / (double) rng.max_value);
        } while ((z == 0) || (z == 1));
        if (z < hotProb) {
            value = hotDistribution->next();
        } else {
            value = hotSetLength + coldDistribution->next();
        }
        return value;
    }

    ~SkewedUniformDistribution() override {
        delete hotDistribution;
        delete coldDistribution;
    }
};


#endif //SETBENCH_SKEWED_UNIFORM_DISTRIBUTION_H
