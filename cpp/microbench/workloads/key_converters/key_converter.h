#ifndef SETBENCH_KEY_CONVERTER_H
#define SETBENCH_KEY_CONVERTER_H

#include <vector>

#include "json/single_include/nlohmann/json.hpp"
#include "workloads/index_maps/index_map.h"
#include "globals_extern.h"

// struct KeyConverter {

//     std::shared_ptr<IndexMap> getIndexMap;
//     std::shared_ptr<IndexMap> insertIndexMap;
//     std::shared_ptr<IndexMap> removeIndexMap;
//     std::shared_ptr<IndexMap> rangeIndexMap;

//     KeyConverter() : getIndexMap(nullptr), insertIndexMap(nullptr), 
//                     removeIndexMap(nullptr), rangeIndexMap(nullptr) {}

//     KeyConverter(std::vector<std::shared_ptr<IndexMap>>&& vec) {
//         if (vec.size() >= 4) {
//             getIndexMap = vec[0];
//             insertIndexMap = vec[1];
//             removeIndexMap = vec[2];
//             rangeIndexMap = vec[3];
//         }
//     }

//     KeyConverter(const KeyConverter&) = default;
//     KeyConverter& operator=(const KeyConverter&) = default;
//     KeyConverter(KeyConverter&&) = default;
//     KeyConverter& operator=(KeyConverter&&) = default;

//     KEY_TYPE * convert(const size_t k, std::string operationType) {
//         if (operationType == "get") {
//             return getIndexMap == nullptr ? nullptr : getIndexMap->getActual(k);
//         } else if (operationType == "insert") {
//             return insertIndexMap == nullptr ? nullptr : insertIndexMap->getActual(k);
//         } else if (operationType == "remove") {
//             return removeIndexMap == nullptr ? nullptr : removeIndexMap->getActual(k);
//         } else if (operationType == "range") {
//             // return rangeIndexMap == nullptr ? nullptr : rangeIndexMap->getActual(k);
//             return rangeIndexMap->getActual(k);
//         }
//         setbench_error("Not possible to convert");
//         // return nullptr;
//     }

//     // virtual std::string toString(size_t indents = 1);

//     // virtual void toJson(nlohmann::json &j) const;

//     // virtual void fromJson(const nlohmann::json &j);

//     ~KeyConverter() = default;
// };

#endif