//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <algorithm>
#include "workloads/data_maps/data_map.h"

class ArrayDataMap : public DataMap<int64_t> {
private:
    int64_t* data_;

public:
    explicit ArrayDataMap(int64_t* data)
        : data_(data) {
    }

    int64_t get(size_t index) override {
        return data_[index];
    }

    ~ArrayDataMap() {
        delete[] data_;
    }
};
