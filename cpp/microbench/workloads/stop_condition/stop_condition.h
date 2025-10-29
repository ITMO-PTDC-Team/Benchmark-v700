//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "json/single_include/nlohmann/json.hpp"

namespace microbench::workload {

struct StopCondition {
    virtual void start(size_t num_threads) = 0;

    /**
     * The purpose of the clean method is to free the resources that the StopCondition may have
     * acquired after it started.
     */
    virtual void clean() {};

    virtual bool is_stopped(int id) = 0;

    virtual std::string to_string(size_t indents = 1) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual ~StopCondition() = default;
};

void to_json(nlohmann::json& j, const StopCondition& s) {
    s.to_json(j);
    assert(j.contains("ClassName"));
    //    assert(j["stopConditionType"] != nullptr);
}

void from_json(const nlohmann::json& j, StopCondition& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
