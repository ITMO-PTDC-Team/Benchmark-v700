#ifndef SETBENCH_FILE_DATA_MAP_H
#define SETBENCH_FILE_DATA_MAP_H

#include "workloads/data_maps/data_map.h"
#include <random>
#include <algorithm>

#ifdef USE_LONG_LONG
class IntDataMap : public DataMap {

public:
    IntDataMap(size_t id) {
        mapId = id;
    }

    void init(size_t range) override {
        actualData = new KEY_TYPE[range];
        for (long long i = 0; i < range; i++) {
            actualData[i] = i + 1;
        }

        //        std::random_shuffle(data, data + range - 1);
        std::shuffle(actualData, actualData + range, std::mt19937(std::random_device()()));
    }

    KEY_TYPE* convert(size_t index) override {
        return &actualData[index];
    }
};
#endif

#endif //SETBENCH_FILE_DATA_MAP_H
