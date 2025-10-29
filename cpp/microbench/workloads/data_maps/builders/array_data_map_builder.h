//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <random>
#include "globals_extern.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/impls/array_data_map.h"

class ArrayDataMapBuilder : public DataMapBuilder {
    int64_t* data_ = nullptr;

public:
    ArrayDataMapBuilder* init(size_t range) override {
        delete[] data_;

        data_ = new int64_t[range];
        for (int64_t i = 0; i < range; i++) {
            data_[i] = i + 1;
        }

        //        std::random_shuffle(data, data + range - 1);
        std::shuffle(data_, data_ + range, std::mt19937(std::random_device()()));
        return this;
    }

    ArrayDataMap* build() override {
        return new ArrayDataMap(data_);
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "ArrayDataMapBuilder";
    }

    void from_json(const nlohmann::json& j) override {
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "ArrayDataMap", indents) +
               indented_title_with_data("ID", id, indents);
    }

    ~ArrayDataMapBuilder() override = default;
};
