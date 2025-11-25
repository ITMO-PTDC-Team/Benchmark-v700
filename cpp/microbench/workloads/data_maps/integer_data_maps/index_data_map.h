#ifndef SETBENCH_INDEX_DATA_MAP_H
#define SETBENCH_INDEX_DATA_MAP_H

#include "workloads/data_maps/data_map.h"
#include <random>
#include <algorithm>

#ifdef USE_LONG_LONG
class IndexDataMap : public DataMap {

public:
    IndexDataMap() {}

    void init(size_t range) override {
        actualData = new KEY_TYPE[range];
        for (long long i = 0; i < range; i++) {
            actualData[i] = i + 1;
        }
    }

    KEY_TYPE* convert(size_t index) override {
        return &actualData[index];
    }

    KEY_TYPE get_min() const override {
        return 0;
    }

    KEY_TYPE get_max(size_t range) const override {
        return range + 1;
    }

    virtual std::string toString(size_t indents = 1) {
        return indented_title_with_str_data("Type", "IndexDataMap", indents);
    }

    virtual DataMap* fromJson(const nlohmann::json& j) override {
        return this;
    }

    virtual void toJson(nlohmann::json &j) const {
        j["dataMap"] = "IndexDataMap";
    }
};
#endif

#endif //SETBENCH_INDEX_DATA_MAP_H
