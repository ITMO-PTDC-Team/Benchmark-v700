//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ID_DATA_MAP_H
#define SETBENCH_ID_DATA_MAP_H

#include "workloads/data_maps/data_map.h"

struct IdDataMap : public DataMap<long long> {
    long long get(size_t index) override {
        return index + 1;
    }
};

#endif //SETBENCH_ID_DATA_MAP_H
