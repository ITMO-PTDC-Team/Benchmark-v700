//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_INDEX_MAP_JSON_CONVECTOR_H
#define SETBENCH_INDEX_MAP_JSON_CONVECTOR_H

#include "json/single_include/nlohmann/json.hpp"
#include "index_map_builder.h"
#include "workloads/index_maps/builders/id_index_map_builder.h"
#include "workloads/index_maps/builders/array_index_map_builder.h"
#include "errors.h"
#include <iostream>

std::map<size_t, IndexMapBuilder *> indexMapBuilders;

IndexMapBuilder *getIndexMapFromJson(const nlohmann::json &j) {
    size_t id = j["id"];

    auto indexMapsBuilderById = indexMapBuilders.find(id);
    if (indexMapsBuilderById != indexMapBuilders.end()) {
        return indexMapsBuilderById->second;
    }

    std::string className = j["ClassName"];
    IndexMapBuilder *indexMapBuilder;
    if (className == "IdIndexMapBuilder") {
        indexMapBuilder = new IdIndexMapBuilder();
    } else if (className == "ArrayIndexMapBuilder") {
        indexMapBuilder = new ArrayIndexMapBuilder();
    } else if (className == "HashIndexMapBuilder") {

    } else {
        setbench_error("JSON PARSER: Unknown class name IndexMapBuilder -- " + className)
    }

    indexMapBuilder->fromJson(j);
    indexMapBuilders.insert({id, indexMapBuilder});
    assert(id + 1 >= IndexMapBuilder::id_counter);
    IndexMapBuilder::id_counter = std::max(IndexMapBuilder::id_counter, id + 1);
    return indexMapBuilder;
}

void deleteIndexMapBuilders() {
    for (auto it: indexMapBuilders) {
        delete it.second;
    }
}

void initIndexMapBuilders(size_t range) {
    for (auto it: indexMapBuilders) {
        it.second->init(range);
    }
}

#endif  // SETBENCH_INDEX_MAP_JSON_CONVECTOR_H
