//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_DATA_MAP_JSON_CONVECTOR_H
#define SETBENCH_DATA_MAP_JSON_CONVECTOR_H

#include "json/single_include/nlohmann/json.hpp"
#include "data_map_builder.h"
#include "data_map_converter.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "errors.h"
#include <iostream>

std::unique_ptr<DataMapConverter> DataMapConverter::instance = nullptr;
std::once_flag DataMapConverter::initFlag;

std::map<size_t, DataMapBuilder *> dataMapBuilders;

DataMapBuilder *getDataMapFromJson(const nlohmann::json &j) {
    size_t id = j["id"];

    auto dataMapsBuilderById = dataMapBuilders.find(id);
    if (dataMapsBuilderById != dataMapBuilders.end()) {
        return dataMapsBuilderById->second;
    }

    std::string className = j["ClassName"];
    DataMapBuilder *dataMapBuilder;
    if (className == "IdDataMapBuilder") {
        dataMapBuilder = new IdDataMapBuilder();
    } else if (className == "ArrayDataMapBuilder") {
        dataMapBuilder = new ArrayDataMapBuilder();
    } else if (className == "HashDataMapBuilder") {

    } else {
        setbench_error("JSON PARSER: Unknown class name DataMapBuilder -- " + className)
    }

    dataMapBuilder->fromJson(j);
    dataMapBuilders.insert({id, dataMapBuilder});
    std::cout << dataMapBuilder->id << " " << id << '\n';
    assert(id + 1 >= DataMapBuilder::id_counter);
    DataMapBuilder::id_counter = std::max(DataMapBuilder::id_counter, id + 1);
    return dataMapBuilder;
}

void deleteDataMapBuilders() {
    for (auto it: dataMapBuilders) {
        delete it.second;
    }
}

void initDataMapBuilders(size_t range) {
    auto& converter = DataMapConverter::getInstance();
    std::cout << "KEK\n";
    for (auto it: dataMapBuilders) {
        it.second->init(range);
        std::cout << it.first << '\n';
        converter.registerData(it.first, it.second->getUnderlyingData());
    }
}

#endif  // SETBENCH_DATA_MAP_JSON_CONVECTOR_H
