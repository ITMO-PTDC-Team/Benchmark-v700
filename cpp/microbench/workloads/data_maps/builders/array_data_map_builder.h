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
    std::vector<long long> data = {};

public:
    ArrayDataMapBuilder* init(size_t range) override {
        data = std::vector<long long>();
        for (long long i = 0; i < range; i++) {
            data.push_back(i + 1);
        }

        //        std::random_shuffle(data, data + range - 1);
        std::shuffle(data.begin(), data.begin() + range, std::mt19937(std::random_device()()));
        return this;
    }

    std::shared_ptr<DataMap<K>> build() override {
        return std::shared_ptr<ArrayDataMap>(new ArrayDataMap(data));
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

    ~ArrayDataMapBuilder() override = default;
};

#endif  // SETBENCH_ARRAY_DATA_MAP_BUILDER_H
