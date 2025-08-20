//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_H
#define SETBENCH_ARGS_GENERATOR_H

#include <utility>

struct ArgsGenerator {
    virtual size_t nextGet() = 0;

    virtual size_t nextInsert() = 0;

    virtual size_t nextRemove() = 0;

    virtual std::pair<size_t, size_t> nextRange() = 0;

    virtual ~ArgsGenerator() = default;
};

#endif //SETBENCH_ARGS_GENERATOR_H
