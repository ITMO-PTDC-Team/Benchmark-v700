//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <utility>

template <typename K>
struct ArgsGenerator {
    virtual K nextGet() = 0;

    virtual K nextInsert() = 0;

    virtual K nextRemove() = 0;

    virtual std::pair<K, K> nextRange() = 0;

    virtual ~ArgsGenerator() = default;
};
