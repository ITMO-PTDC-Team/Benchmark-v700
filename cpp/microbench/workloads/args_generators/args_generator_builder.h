//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_BUILDER_H
#define SETBENCH_ARGS_GENERATOR_BUILDER_H

#include <string>
#include "args_generator.h"
#include "random_xoshiro256p.h"
#include "json/single_include/nlohmann/json.hpp"

typedef long long K;

//template<typename K>
struct ArgsGeneratorBuilder {
    virtual ArgsGeneratorBuilder * init(size_t range) = 0;

    virtual ArgsGenerator<K> * build(Random64 & _rng) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual std::string toString(size_t indents = 1) = 0;

    virtual ~ArgsGeneratorBuilder() = default;
};

void to_json(nlohmann::json &j, const ArgsGeneratorBuilder &s) {
    s.toJson(j);
    assert(j.contains("ClassName"));
//    assert(j["argsGeneratorType"] != nullptr);
}

void from_json(const nlohmann::json &j, ArgsGeneratorBuilder &s) {
    s.fromJson(j);
}

#endif //SETBENCH_ARGS_GENERATOR_BUILDER_H
