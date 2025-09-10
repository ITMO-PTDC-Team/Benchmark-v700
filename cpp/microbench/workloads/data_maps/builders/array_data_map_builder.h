//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ARRAY_DATA_MAP_BUILDER_H
#define SETBENCH_ARRAY_DATA_MAP_BUILDER_H

#include <random>
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/impls/id_data_map.h"
#include "workloads/data_maps/impls/array_data_map.h"

class ArrayDataMapBuilder : public DataMapBuilder {
    long long* data = nullptr;
    long long* reverseData = nullptr;
    KEY_TYPE* actualData = nullptr;

public:
    ArrayDataMapBuilder* init(size_t range) override {
        delete[] data;
        delete[] reverseData;
        delete[] actualData;

        data = new long long[range];
        reverseData = new long long[range];
        actualData = new KEY_TYPE[range];
        for (long long i = 0; i < range; i++) {
            data[i] = i + 1;
        }

        //        std::random_shuffle(data, data + range - 1);
        std::shuffle(data, data + range, std::mt19937(std::random_device()()));
        for (long long i = 0; i < range; i++) {
            actualData[i] = data[i];
            reverseData[data[i] - 1] = i;
        }
        return this;
    }

    ArrayDataMap* build() override {
        return new ArrayDataMap(data, reverseData, id);
    }

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "ArrayDataMapBuilder";
    }

    void fromJson(const nlohmann::json& j) override {
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "ArrayDataMap", indents) +
               indented_title_with_data("ID", id, indents);
    }

    KEY_TYPE* getUnderlyingData() override {
        return actualData;
    };

    ~ArrayDataMapBuilder() override = default;
};

#endif  // SETBENCH_ARRAY_DATA_MAP_BUILDER_H
