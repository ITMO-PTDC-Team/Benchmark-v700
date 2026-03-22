//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <memory>
#include <string>
#include "args_generator.h"
#include "nlohmann/json.hpp"
#include "random_xoshiro256p.h"

namespace microbench::workload {

using KeyType = int64_t;

struct ArgsGeneratorBuilder {
    virtual ArgsGeneratorBuilder& init(size_t range) = 0;

    virtual ArgsGeneratorPtr build(Random64& rng) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual std::string to_string(size_t indents = 1) = 0;

    virtual ~ArgsGeneratorBuilder() = default;
};

using ArgsGeneratorBuilderPtr = std::shared_ptr<ArgsGeneratorBuilder>;

void to_json(nlohmann::json& j, const ArgsGeneratorBuilder& s) {
    s.to_json(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json& j, ArgsGeneratorBuilder& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
