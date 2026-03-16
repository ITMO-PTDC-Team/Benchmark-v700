//
// Created by Ravil Galiev on 27.07.2023.
//
#pragma once

#include "data_map_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "errors.h"

namespace microbench::workload {

std::map<size_t, DataMapBuilderPtr> data_map_builders;

DataMapBuilderPtr get_data_map_from_json(const nlohmann::json& j) {
    size_t id = j["id"];

    auto data_maps_builder_by_id = data_map_builders.find(id);
    if (data_maps_builder_by_id != data_map_builders.end()) {
        return data_maps_builder_by_id->second;
    }

    std::string class_name = j["ClassName"];
    DataMapBuilderPtr data_map_builder;
    if (class_name == "IdDataMapBuilder") {
        data_map_builder = std::make_shared<IdDataMapBuilder>();
    } else if (class_name == "ArrayDataMapBuilder") {
        data_map_builder = std::make_shared<ArrayDataMapBuilder>();
    } else if (class_name == "HashDataMapBuilder") {
        setbench_error("HashDataMapBuilder unimplemented")
    } else {
        setbench_error("JSON PARSER: Unknown class name DataMapBuilder -- " + class_name)
    }

    data_map_builder->from_json(j);
    data_map_builders.insert({id, data_map_builder});
    DataMapBuilder::id_counter = std::max(DataMapBuilder::id_counter, id + 1);
    return data_map_builder;
}

void init_data_map_builders(size_t range) {
    for (auto it : data_map_builders) {
        it.second->init(range);
    }
}

}  // namespace microbench::workload
