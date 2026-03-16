//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <cstddef>
#include <memory>

namespace microbench::workload {

struct Distribution {
    virtual size_t next() = 0;

    virtual ~Distribution() = default;
};

using DistributionPtr = std::unique_ptr<Distribution>;

struct MutableDistribution : public Distribution {
    virtual void set_range(size_t range) = 0;

    virtual size_t next() = 0;

    size_t next(size_t range) {
        set_range(range);
        return this->next();
    }
};

using MutableDistributionPtr = std::unique_ptr<MutableDistribution>;

}  // namespace microbench::workload
