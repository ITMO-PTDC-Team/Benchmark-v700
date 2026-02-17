//
// Created by Ravil Galiev on 24.07.2023.
//
#pragma once

#include <string>
#include "data_map.h"
#include "nlohmann/json.hpp"

namespace microbench::workload {

using K = int64_t;

struct DataMapBuilder {
    static size_t id_counter;

    const size_t id = id_counter++;

    virtual DataMapBuilder* init(size_t range) = 0;

    virtual DataMap<K>* build() = 0;

    virtual std::string to_string(size_t indents = 1) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual ~DataMapBuilder() = default;
};

size_t DataMapBuilder::id_counter = 0;

void to_json(nlohmann::json& j, const DataMapBuilder& s) {
    s.to_json(j);
    j["id"] = s.id;
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json& j, DataMapBuilder& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
