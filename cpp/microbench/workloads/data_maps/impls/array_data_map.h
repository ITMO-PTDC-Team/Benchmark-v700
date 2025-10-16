//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <algorithm>
#include "workloads/data_maps/data_map.h"

class ArrayDataMap : public DataMap<long long> {
private:
    long long *data;
public:

    ArrayDataMap(long long int *data) : data(data) {}

    long long get(size_t index) override {
        return data[index];
    }

    ~ArrayDataMap() {
        delete[] data;
    }

};
