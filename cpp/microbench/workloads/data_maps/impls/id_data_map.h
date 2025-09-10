//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ID_DATA_MAP_H
#define SETBENCH_ID_DATA_MAP_H

#include "workloads/data_maps/data_map.h"

class IdDataMap : public DataMap {
// private:
//     KEY_TYPE* actualData;

public:
    IdDataMap(size_t id) {
        // assert(sizeof(actualData) == sizeof(KEY_TYPE));
        mapId = id;
    }

    long long get(size_t index) override {
        return index + 1;
    }

    KEY_TYPE* getActual(size_t index) override {
        auto& converter = DataMapConverter::getInstance();
        converter.set(mapId, 0, index);
        return converter.convert(mapId, 0);
    }
};

#endif //SETBENCH_ID_DATA_MAP_H
