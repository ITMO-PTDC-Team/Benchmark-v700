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
            char c = 'a';
            result = c + result;
        }
        return result;
    }


    void init(size_t range) override {
        actualData = new KEY_TYPE[range];
        for (long long i = 0; i < range; i++) {
            actualData[i] = generate_string_key(i + 1);
        }
    }

    KEY_TYPE* convert(size_t index) override {
        return &actualData[index];
    }

    KEY_TYPE get_min() {
        return "";
    }

    KEY_TYPE get_max(size_t range) {
        std::string mx_val;
        for (int i = 0; i<range; ++i) {
            mx_val += "a";
        }
        return mx_val;
    }

    virtual std::string toString(size_t indents = 1) {
        return indented_title_with_str_data("Type", "StringDataMap", indents);
    }

    virtual void toJson(nlohmann::json &j) const {
        j["dataMap"] = "StringDataMap";
    }
    
#endif
};

#endif //SETBENCH_STRING_DATA_MAP_H
