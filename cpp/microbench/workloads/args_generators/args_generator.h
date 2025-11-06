//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_H
#define SETBENCH_ARGS_GENERATOR_H

#include <utility>

template<typename K>
struct ArgsGenerator {
    virtual K nextGet() = 0;

    virtual K nextInsert() = 0;

    virtual K nextRemove() = 0;

    virtual std::pair<K, K> nextRange() = 0;

    virtual ~ArgsGenerator() = default;
};

#endif //SETBENCH_ARGS_GENERATOR_H
