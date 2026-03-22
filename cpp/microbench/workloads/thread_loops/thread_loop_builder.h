//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <memory>
#include <string>
#include "random_xoshiro256p.h"
#include "stop_condition/stop_condition.h"
#include "thread_loop.h"
#include "globals_t.h"

namespace microbench::workload {

struct ThreadLoopBuilder {
    size_t RQ_RANGE;

    virtual ThreadLoopBuilder& init(int range) {
        RQ_RANGE = range;
        return *this;
    };

    virtual ThreadLoopPtr build(globals_t* g, Random64& rng, size_t tid,
                                StopConditionPtr stop_condition) = 0;

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

using ThreadLoopBuilderPtr = std::shared_ptr<ThreadLoopBuilder>;

}  // namespace microbench::workload
