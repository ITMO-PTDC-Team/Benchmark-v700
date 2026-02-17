//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <string>
#include "random_xoshiro256p.h"
#include "thread_loop.h"
#include "nlohmann/json.hpp"
#include "globals_t.h"

namespace microbench::workload {

struct ThreadLoopBuilder {
    size_t RQ_RANGE;

    virtual ThreadLoopBuilder* init(int range) {
        RQ_RANGE = range;
        return this;
    };

    virtual ThreadLoop* build(globals_t* g, Random64& rng, size_t tid,
                              StopCondition* stop_condition) = 0;

    virtual void to_json(nlohmann::json& j) const = 0;

    virtual void from_json(const nlohmann::json& j) = 0;

    virtual std::string to_string(size_t indents = 1) = 0;

    virtual ~ThreadLoopBuilder() = default;
};

void to_json(nlohmann::json& j, const ThreadLoopBuilder& s) {
    s.to_json(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json& j, ThreadLoopBuilder& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
