//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ID_DATA_MAP_BUILDER_H
#define SETBENCH_ID_DATA_MAP_BUILDER_H

#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/impls/id_data_map.h"
#include "globals_extern.h"

struct IdDataMapBuilder : public DataMapBuilder {
    KEY_TYPE* actualData;

    IdDataMapBuilder* init(size_t range) override {
        delete[] actualData;
        actualData = new KEY_TYPE[1];

        return this;
    };

    IdDataMap* build() override {
        return new IdDataMap(id);
    };

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "IdDataMapBuilder";
    }

    void fromJson(const nlohmann::json& j) override {
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "IdDataMap", indents) +
               indented_title_with_data("ID", id, indents);
    }

    KEY_TYPE* getUnderlyingData() override {
        return actualData;
    };

    ~IdDataMapBuilder() override = default;
};

#endif  // SETBENCH_ID_DATA_MAP_BUILDER_H
