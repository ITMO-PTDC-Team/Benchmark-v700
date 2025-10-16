//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

template <typename K>
struct DataMap {
    virtual K get(size_t index) = 0;

    virtual ~DataMap() = default;
};
