//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ID_INDEX_MAP_H
#define SETBENCH_ID_INDEX_MAP_H

#include "workloads/index_maps/index_map.h"

class IdIndexMap : public IndexMap {

public:
    IdIndexMap(size_t id) {
        // assert(sizeof(actualData) == sizeof(KEY_TYPE));
        mapId = id;
    }

    long long get(size_t index) override {
        return index + 1;
    }
};

#endif //SETBENCH_ID_INDEX_MAP_H
