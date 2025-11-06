//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_DATA_MAP_BUILDER_H
#define SETBENCH_DATA_MAP_BUILDER_H

#include <string>
#include "data_map.h"
#include "json/single_include/nlohmann/json.hpp"

typedef long long K;

//template<typename K>
struct DataMapBuilder {
    static size_t id_counter;

    const size_t id = id_counter++;

    virtual DataMapBuilder *init(size_t range) = 0;

    virtual DataMap<K> *build() = 0;

    virtual std::string toString(size_t indents = 1) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual ~DataMapBuilder() = default;
};

size_t DataMapBuilder::id_counter = 0;

void to_json(nlohmann::json &j, const DataMapBuilder &s) {
    s.toJson(j);
    j["id"] = s.id;
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json &j, DataMapBuilder &s) {
    s.fromJson(j);
}

#endif //SETBENCH_DATA_MAP_BUILDER_H
