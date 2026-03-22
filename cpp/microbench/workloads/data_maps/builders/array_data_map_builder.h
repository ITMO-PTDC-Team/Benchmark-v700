//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <memory>
#include <random>
#include "data_maps/data_map.h"
#include "globals_extern.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/impls/array_data_map.h"

namespace microbench::workload {

class ArrayDataMapBuilder : public DataMapBuilder {
    std::vector<int64_t> data_;

public:
    ArrayDataMapBuilder& init(size_t range) override {
        data_.clear();
        data_.resize(range);
        for (int64_t i = 0; i < range; i++) {
            data_[i] = i + 1;
        }
        std::shuffle(data_.begin(), data_.end(), std::mt19937(std::random_device()()));
        return *this;
    }

    DataMapPtr build() override {
        return std::make_shared<ArrayDataMap>(data_);
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

}  // namespace microbench::workload
