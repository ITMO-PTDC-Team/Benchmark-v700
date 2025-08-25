//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_DATA_MAP_JSON_CONVECTOR_H
#define SETBENCH_DATA_MAP_JSON_CONVECTOR_H

#include "json/single_include/nlohmann/json.hpp"
#include "data_map_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "errors.h"

std::map<size_t, std::shared_ptr<DataMapBuilder>> dataMapBuilders;

std::shared_ptr<DataMapBuilder> getDataMapFromJson(const nlohmann::json &j) {
    size_t id = j["id"];

    auto dataMapsBuilderById = dataMapBuilders.find(id);
    if (dataMapsBuilderById != dataMapBuilders.end()) {
        return dataMapsBuilderById->second;
    }

    std::string className = j["ClassName"];
    std::shared_ptr<DataMapBuilder> dataMapBuilder;
    if (className == "IdDataMapBuilder") {
        dataMapBuilder = std::make_shared<IdDataMapBuilder>();
    } else if (className == "ArrayDataMapBuilder") {
            dataMapBuilder = std::make_shared<ArrayDataMapBuilder>();
    } else if (className == "HashDataMapBuilder") {

    } else {
        setbench_error("JSON PARSER: Unknown class name DataMapBuilder -- " + className)
    }

    dataMapBuilder->fromJson(j);
    dataMapBuilders.insert({id, dataMapBuilder});
    DataMapBuilder::id_counter = std::max(DataMapBuilder::id_counter, id + 1);
    return dataMapBuilder;
}

void deleteDataMapBuilders() {
    // noop since smart pointers used
}

void initDataMapBuilders(size_t range) {
    for (auto it: dataMapBuilders) {
        it.second->init(range);
    }
}

#endif  // SETBENCH_DATA_MAP_JSON_CONVECTOR_H
