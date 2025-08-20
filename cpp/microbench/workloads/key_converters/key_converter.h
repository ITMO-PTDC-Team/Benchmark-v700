#ifndef SETBENCH_KEY_CONVERTER_H
#define SETBENCH_KEY_CONVERTER_H

#include "json/single_include/nlohmann/json.hpp"
#include "globals_extern.h"

struct KeyConverter {
    KeyConverter() = default;

    KEY_TYPE * convert(const size_t k) {

    }

    virtual std::string toString(size_t indents = 1) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual ~KeyConverter() = default;
};

#endif