//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/impls/id_data_map.h"
#include "globals_extern.h"

namespace microbench::workload {

struct IdDataMapBuilder : public DataMapBuilder {
    IdDataMapBuilder* init(size_t range) override {
        return this;
    };

    IdDataMap* build() override {
        return new IdDataMap();
    };

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "IdDataMapBuilder";
    }

    void from_json(const nlohmann::json& j) override {
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "IdDataMap", indents) +
               indented_title_with_data("ID", id, indents);
    }

    ~IdDataMapBuilder() override = default;
};

}  // namespace microbench::workload
