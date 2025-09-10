#ifndef DATA_MAP_CONVERTER_H
#define DATA_MAP_CONVERTER_H

#include <vector>
#include <mutex>

#include "json/single_include/nlohmann/json.hpp"
#include "globals_extern.h"
#include <iostream>

struct DataMapConverter {
    static std::unique_ptr<DataMapConverter> instance;
    static std::once_flag initFlag;

    std::unordered_map<size_t, KEY_TYPE*> dataArrays;
    
    DataMapConverter() = default;
    ~DataMapConverter() = default;

    static DataMapConverter& getInstance() {
        static std::once_flag onceFlag;
        std::call_once(onceFlag, []() {
            instance.reset(new DataMapConverter());
        });
        return *instance;
    }
    
    void registerData(size_t id, KEY_TYPE* actualData) {
        dataArrays[id] = actualData;
    }

    KEY_TYPE* getDataMap(size_t mapId) {
        auto it = dataArrays.find(mapId);
        if (it != dataArrays.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    KEY_TYPE* convert(size_t sourceMapId, size_t index) {
        auto sourceData = getDataMap(sourceMapId - 1);
        if (sourceData) {
            KEY_TYPE* actualValue = &sourceData[index];
            return actualValue;
        }
        setbench_error("Actual data not found for DataMap with id: " + sourceMapId);
    }

    void set(size_t sourceMapId, size_t index, KEY_TYPE key) {
        auto sourceData = getDataMap(sourceMapId - 1);
        if (sourceData) {
            sourceData[index] = key;
            return;
        }
        setbench_error("Was unable to set for DataMap with id: " + sourceMapId);
    }

    // DataMapConverter(const DataMapConverter&) = default;
    // DataMapConverter& operator=(const DataMapConverter&) = default;
    // DataMapConverter(DataMapConverter&&) = default;
    // DataMapConverter& operator=(DataMapConverter&&) = default;

    // virtual std::string toString(size_t indents = 1);

    // virtual void toJson(nlohmann::json &j) const;

    // virtual void fromJson(const nlohmann::json &j);

    // ~DataMapConverter() = default;
};

#endif