//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_ID_INDEX_MAP_BUILDER_H
#define SETBENCH_ID_INDEX_MAP_BUILDER_H

#include "workloads/index_maps/index_map_builder.h"
#include "workloads/index_maps/impls/id_index_map.h"
#include "globals_extern.h"

struct IdIndexMapBuilder : public IndexMapBuilder {
    KEY_TYPE* actualData;

    IdIndexMapBuilder* init(size_t range) override {
        delete[] actualData;
        actualData = new KEY_TYPE[1];

        return this;
    };

    IdIndexMap* build() override {
        return new IdIndexMap(id);
    };

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "IdIndexMapBuilder";
    }

    void fromJson(const nlohmann::json& j) override {
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "IdIndexMap", indents) +
               indented_title_with_data("ID", id, indents);
    }

    ~IdIndexMapBuilder() override = default;
};

#endif  // SETBENCH_ID_INDEX_MAP_BUILDER_H
