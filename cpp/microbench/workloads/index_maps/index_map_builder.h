//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_INDEX_MAP_BUILDER_H
#define SETBENCH_INDEX_MAP_BUILDER_H

#include <string>
#include "index_map.h"
#include "json/single_include/nlohmann/json.hpp"

//template<typename K>
struct IndexMapBuilder {
    static size_t id_counter;

    const size_t id = id_counter++;

    virtual IndexMapBuilder *init(size_t range) = 0;

    virtual IndexMap* build() = 0;

    virtual std::string toString(size_t indents = 1) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual ~IndexMapBuilder() = default;
};

size_t IndexMapBuilder::id_counter = 0;

void to_json(nlohmann::json &j, const IndexMapBuilder &s) {
    s.toJson(j);
    j["id"] = s.id;
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json &j, IndexMapBuilder &s) {
    s.fromJson(j);
}

#endif //SETBENCH_INDEX_MAP_BUILDER_H
