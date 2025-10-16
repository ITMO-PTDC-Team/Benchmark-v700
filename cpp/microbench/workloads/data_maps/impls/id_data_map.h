//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include "workloads/data_maps/data_map.h"

struct IdDataMap : public DataMap<long long> {
    long long get(size_t index) override {
        return index + 1;
    }
};
