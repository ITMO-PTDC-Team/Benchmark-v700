//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ARRAY_INDEX_MAP_H
#define SETBENCH_ARRAY_INDEX_MAP_H

#include <algorithm>
#include "workloads/index_maps/index_map.h"

class ArrayIndexMap : public IndexMap {
private:
    size_t *data;
public:

    ArrayIndexMap(size_t *data) : data(data) {
    }

    size_t get(size_t index) override {
        return data[index];
    }

    ~ArrayIndexMap() {
        delete[] data;
    }

};

#endif //SETBENCH_ARRAY_INDEX_MAP_H
