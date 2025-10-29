//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include "workloads/data_maps/data_map.h"

struct IdDataMap : public DataMap<int64_t> {
    int64_t get(size_t index) override {
        return index + 1;
    }
};
