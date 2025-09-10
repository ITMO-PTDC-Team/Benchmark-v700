#ifndef SETBENCH_KEY_CONVERTER_H
#define SETBENCH_KEY_CONVERTER_H

#include <vector>

#include "json/single_include/nlohmann/json.hpp"
#include "workloads/data_maps/data_map.h"
#include "globals_extern.h"

struct KeyConverter {

    std::shared_ptr<DataMap> getDataMap;
    std::shared_ptr<DataMap> insertDataMap;
    std::shared_ptr<DataMap> removeDataMap;
    std::shared_ptr<DataMap> rangeDataMap;

    KeyConverter() : getDataMap(nullptr), insertDataMap(nullptr), 
                    removeDataMap(nullptr), rangeDataMap(nullptr) {}

    KeyConverter(std::vector<std::shared_ptr<DataMap>>&& vec) {
        if (vec.size() >= 4) {
            getDataMap = vec[0];
            insertDataMap = vec[1];
            removeDataMap = vec[2];
            rangeDataMap = vec[3];
        }
    }

    KeyConverter(const KeyConverter&) = default;
    KeyConverter& operator=(const KeyConverter&) = default;
    KeyConverter(KeyConverter&&) = default;
    KeyConverter& operator=(KeyConverter&&) = default;

    KEY_TYPE * convert(const size_t k, std::string operationType) {
        if (operationType == "get") {
            return getDataMap == nullptr ? nullptr : getDataMap->getActual(k);
        } else if (operationType == "insert") {
            return insertDataMap == nullptr ? nullptr : insertDataMap->getActual(k);
        } else if (operationType == "remove") {
            return removeDataMap == nullptr ? nullptr : removeDataMap->getActual(k);
        } else if (operationType == "range") {
            // return rangeDataMap == nullptr ? nullptr : rangeDataMap->getActual(k);
            return rangeDataMap->getActual(k);
        }
        setbench_error("Not possible to convert");
        // return nullptr;
    }

    // virtual std::string toString(size_t indents = 1);

    // virtual void toJson(nlohmann::json &j) const;

    // virtual void fromJson(const nlohmann::json &j);

    ~KeyConverter() = default;
};

#endif