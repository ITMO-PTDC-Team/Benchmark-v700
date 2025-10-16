//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

struct Distribution {
    virtual size_t next() = 0;

    virtual ~Distribution() = default;
};

struct MutableDistribution : public Distribution {
    virtual void setRange(size_t range) = 0;

    virtual size_t next() = 0;

    size_t next(size_t range) {
        setRange(range);
        return this->next();
    }
};
