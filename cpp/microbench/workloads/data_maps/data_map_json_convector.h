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

class BaseDataMapBuilderFactory {
    public:
        virtual ~BaseDataMapBuilderFactory() = default;
        virtual DataMapBuilder* create() = 0;
};
    
template <typename DataMapBuilder>
class DataMapBuilderFactory : public BaseDataMapBuilderFactory {
    public:
    DataMapBuilder *create() override {
        return new DataMapBuilder();
    }
};

#define REGISTER_DATA_MAP_BUILDER(className) \
    map.insert({#className, std::make_unique<DataMapBuilderFactory<className>>()})

std::map<size_t, DataMapBuilder *> dataMapBuilders;
inline static std::map<std::string, std::unique_ptr<BaseDataMapBuilderFactory>> dataMapFactoryMap = [] {
    std::map<std::string, std::unique_ptr<BaseDataMapBuilderFactory>> map;
    REGISTER_DATA_MAP_BUILDER(IdDataMapBuilder);
    REGISTER_DATA_MAP_BUILDER(ArrayDataMapBuilder);
    //REGISTER_DATA_MAP_BUILDER(HashDataMapBuilder);
    return map;
}();

DataMapBuilder *getDataMapFromJson(const nlohmann::json &j) {
    size_t id = j["id"];

    auto dataMapsBuilderById = dataMapBuilders.find(id);
    if (dataMapsBuilderById != dataMapBuilders.end()) {
        return dataMapsBuilderById->second;
    }

    std::string className = j["ClassName"];
    if (dataMapFactoryMap.find(className) != dataMapFactoryMap.end()) {
        DataMapBuilder *dataMapBuilder = dataMapFactoryMap[className]->create();
        dataMapBuilder->fromJson(j);
        dataMapBuilders.insert({id, dataMapBuilder});
        DataMapBuilder::id_counter = std::max(DataMapBuilder::id_counter, id + 1);
        return dataMapBuilder;
    }
    setbench_error("JSON PARSER: Unknown class name DataMapBuilder -- " + className)
}

void deleteDataMapBuilders() {
    for (auto it: dataMapBuilders) {
        delete it.second;
    }
}

void initDataMapBuilders(size_t range) {
    for (auto it: dataMapBuilders) {
        it.second->init(range);
    }
}

#endif  // SETBENCH_DATA_MAP_JSON_CONVECTOR_H
