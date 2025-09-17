//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_INDEX_MAP_H
#define SETBENCH_INDEX_MAP_H

#include "index_map_converter.h"

class IndexMap {
protected:
    size_t mapId;

public:
    virtual long long get(size_t index) = 0;

    virtual ~IndexMap() = default;

    virtual size_t getId() const { return mapId; }
};

#endif //SETBENCH_INDEX_MAP_H
