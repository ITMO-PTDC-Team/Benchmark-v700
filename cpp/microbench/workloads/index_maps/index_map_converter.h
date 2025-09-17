#ifndef INDEX_MAP_CONVERTER_H
#define INDEX_MAP_CONVERTER_H

#include <vector>
#include <mutex>

#include "json/single_include/nlohmann/json.hpp"
#include "globals_extern.h"

// struct IndexMapConverter {
//     static std::unique_ptr<IndexMapConverter> instance;
//     static std::once_flag initFlag;

//     std::unordered_map<size_t, KEY_TYPE*> dataArrays;
    
//     IndexMapConverter() = default;
//     ~IndexMapConverter() = default;

//     static IndexMapConverter& getInstance() {
//         static std::once_flag onceFlag;
//         std::call_once(onceFlag, []() {
//             instance.reset(new IndexMapConverter());
//         });
//         return *instance;
//     }
    
//     void registerIndex(size_t id, KEY_TYPE* actualIndex) {
//         dataArrays[id] = actualIndex;
//     }

//     KEY_TYPE* getIndexMap(size_t mapId) {
//         auto it = dataArrays.find(mapId);
//         if (it != dataArrays.end()) {
//             return it->second;
//         }
//         return nullptr;
//     }
    
//     KEY_TYPE* convert(size_t sourceMapId, size_t index) {
//         auto sourceIndex = getIndexMap(sourceMapId - 1);
//         if (sourceIndex) {
//             KEY_TYPE* actualValue = &sourceIndex[index];
//             return actualValue;
//         }
//         setbench_error("Actual data not found for IndexMap with id: " + sourceMapId);
//     }

//     void set(size_t sourceMapId, size_t index, KEY_TYPE key) {
//         auto sourceIndex = getIndexMap(sourceMapId - 1);
//         if (sourceIndex) {
//             sourceIndex[index] = key;
//             return;
//         }
//         setbench_error("Was unable to set for IndexMap with id: " + sourceMapId);
//     }

//     // IndexMapConverter(const IndexMapConverter&) = default;
//     // IndexMapConverter& operator=(const IndexMapConverter&) = default;
//     // IndexMapConverter(IndexMapConverter&&) = default;
//     // IndexMapConverter& operator=(IndexMapConverter&&) = default;

//     // virtual std::string toString(size_t indents = 1);

//     // virtual void toJson(nlohmann::json &j) const;

//     // virtual void fromJson(const nlohmann::json &j);

//     // ~IndexMapConverter() = default;
// };

#endif