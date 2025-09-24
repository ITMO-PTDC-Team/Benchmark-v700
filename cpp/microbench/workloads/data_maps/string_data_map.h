#ifndef SETBENCH_STRING_DATA_MAP_H
#define SETBENCH_STRING_DATA_MAP_H

#include "workloads/data_maps/data_map.h"
#include <random>
#include <algorithm>
#include <string>

class StringDataMap : public DataMap {

#ifdef USE_STRING
public:
    StringDataMap(size_t id) {
        mapId = id;
    }

    std::string generate_string_key(long long index) {
        std::string result;
        while (index > 0) {
            index--;
            char c = 'a' + (index % 26);
            result = c + result;
            index /= 26;
        }
        return result;
    }


    void init(size_t range) override {
        actualData = new KEY_TYPE[range];
        for (long long i = 0; i < range; i++) {
            actualData[i] = generate_string_key(i + 1);
        }

        //        std::random_shuffle(data, data + range - 1);
        // std::shuffle(actualData, actualData + range, std::mt19937(std::random_device()()));
    }

    KEY_TYPE* convert(size_t index) override {
        return nullptr;
    }
#endif
};

#endif //SETBENCH_STRING_DATA_MAP_H
