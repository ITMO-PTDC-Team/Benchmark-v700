//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ARRAY_INDEX_MAP_BUILDER_H
#define SETBENCH_ARRAY_INDEX_MAP_BUILDER_H

#include <random>
#include "workloads/index_maps/index_map_builder.h"
#include "workloads/index_maps/impls/id_index_map.h"
#include "workloads/index_maps/impls/array_index_map.h"

class ArrayIndexMapBuilder : public IndexMapBuilder {
    size_t* data = nullptr;

public:
    ArrayIndexMapBuilder* init(size_t range) override {
        delete[] data;
        data = new size_t[range];
        for (size_t i = 0; i < range; i++) {
            data[i] = i + 1;
        }

        //        std::random_shuffle(data, data + range - 1);
        std::shuffle(data, data + range, std::mt19937(std::random_device()()));
        return this;
    }

    ArrayIndexMap* build() override {
        return new ArrayIndexMap(data);
    }

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "ArrayIndexMapBuilder";
    }

    void fromJson(const nlohmann::json& j) override {
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "ArrayIndexMap", indents) +
               indented_title_with_data("ID", id, indents);
    }
    
    ~ArrayIndexMapBuilder() override = default;
};

#endif  // SETBENCH_ARRAY_INDEX_MAP_BUILDER_H
