//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_ZIPF_DISTRIBUTION_H
#define SETBENCH_ZIPF_DISTRIBUTION_H

#include <algorithm>
#include <cassert>
#include "random_xoshiro256p.h"
#include "plaf.h"
#include "workloads/distributions/distribution.h"

class ZipfDistribution : public MutableDistribution {
private:
    PAD;
    Random64 &rng;
    size_t last_range;
    double area;
    double alpha;
    PAD;
public:

    ZipfDistribution(Random64 &_rng, double _alpha = 1.0, size_t _range = 0)
            : rng(_rng), alpha(_alpha) {
        setRange(_range);
    }

    void setRange(size_t range) override {
        if (last_range == range)
            return;

        last_range = range;

        ++range;
        if (alpha == 1.0) {
            area = log(range);
        } else {
            area = (pow((double) range, 1.0 - alpha) - 1.0) / (1.0 - alpha);
        }
    }

    size_t next() override {
        double z; // Uniform random number (0 < z < 1)
        do {
            z = (rng.next() / (double) rng.max_value);
        } while ((z == 0) || (z == 1));
        size_t zipf_value = 0;
        double s = area * z;
        if (alpha == 1.0) {
            zipf_value = (size_t) exp(s);
        } else {
            zipf_value = (size_t) pow(s * (1.0 - alpha) + 1.0, 1.0 / (1.0 - alpha));
        }
        return zipf_value - 1;
    }

    ~ZipfDistribution() override = default;
};


#endif //SETBENCH_ZIPF_DISTRIBUTION_H
