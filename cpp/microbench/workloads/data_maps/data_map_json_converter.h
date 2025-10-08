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
    std::string className = j["name"];
    #ifdef USE_LONG_LONG
        #include "int_data_map.h"
        if (className == "IntDataMap") {
            return (new IntDataMap())->fromJson(j);
        };

        setbench_error("CMake and JSON data map name is not equal")
    #endif
        
    #ifdef USE_STRING
        #include "string_data_map.h"
        if (className == "StringDataMap") {
            return (new StringDataMap())->fromJson(j);
        };

        setbench_error("CMake and JSON data map name is not equal")
    #endif

    return nullptr;
}

#endif  // SETBENCH_DATA_MAP_JSON_CONVERTER_H
