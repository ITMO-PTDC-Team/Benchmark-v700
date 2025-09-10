//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_DATA_MAP_H
#define SETBENCH_DATA_MAP_H

#include "data_map_converter.h"

class DataMap {
protected:
    size_t mapId;

public:
    virtual long long get(size_t index) = 0;

    virtual KEY_TYPE* getActual(size_t index) = 0;

    virtual ~DataMap() = default;

    virtual size_t getId() const { return mapId; }
};

#endif //SETBENCH_DATA_MAP_H
