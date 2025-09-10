//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ARRAY_DATA_MAP_H
#define SETBENCH_ARRAY_DATA_MAP_H

#include <algorithm>
#include "workloads/data_maps/data_map.h"

class ArrayDataMap : public DataMap<long long> {
private:
    long long *reverseData;
    long long *data;
public:

    ArrayDataMap(long long int *data, long long int* reverseData, size_t id) : data(data), reverseData(reverseData) {
        mapId = id;
    }

    long long get(size_t index) override {
        return data[index];
    }

    long long* getActual(size_t index) override {
        auto& converter = DataMapConverter::getInstance();
        return converter.convert(mapId, reverseData[index - 1]);
    }

    ~ArrayDataMap() {
        delete[] data;
    }

};

#endif //SETBENCH_ARRAY_DATA_MAP_H
