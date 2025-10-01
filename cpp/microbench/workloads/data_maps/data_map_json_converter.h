//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_DATA_MAP_JSON_CONVERTER_H
#define SETBENCH_DATA_MAP_JSON_CONVERTER_H

#include "json/single_include/nlohmann/json.hpp"
#include "data_map.h"
#include "errors.h"

#include <iostream>
#include <type_traits>

DataMap* getDataMapFromJson(const nlohmann::json &j) {
    // size_t id = j["id"];

    // auto indexMapsBuilderById = indexMapBuilders.find(id);
    // if (indexMapsBuilderById != indexMapBuilders.end()) {
    //     return indexMapsBuilderById->second;
    // }

    // std::string className = j["dataMap"];
    if constexpr (std::is_same_v<KEY_TYPE, std::string>) {
        #ifdef USE_STRING
        #include "string_data_map.h"
        return new StringDataMap(1);
        #endif
    } else if constexpr (std::is_same_v<KEY_TYPE, long long>) {
        #ifdef USE_LONG_LONG
        #include "int_data_map.h"
        return new IntDataMap(1);
        #endif
    }

    // setbench_error("JSON PARSER: Unknown class name DataMap -- " + className)

    // TODO: In case we want additional functionality for DataMaps
    // indexMapBuilder->fromJson(j);
    return nullptr;
}

#endif  // SETBENCH_DATA_MAP_JSON_CONVERTER_H
