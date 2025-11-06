//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_THREAD_LOOP_BUILDER_H
#define SETBENCH_THREAD_LOOP_BUILDER_H

#include <string>
#include "thread_loop.h"
#include "json/single_include/nlohmann/json.hpp"
#include "globals_t.h"

//template<typename K>
struct ThreadLoopBuilder {
    size_t RQ_RANGE;

    virtual ThreadLoopBuilder *init(int range) {
        RQ_RANGE = range;
        return this;
    };

    virtual ThreadLoop *build(globals_t *_g, Random64 & _rng, size_t _tid, StopCondition *_stopCondition) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;

    virtual void fromJson(const nlohmann::json &j) = 0;

    virtual std::string toString(size_t indents = 1) = 0;

    virtual ~ThreadLoopBuilder() = default;
};

void to_json(nlohmann::json &j, const ThreadLoopBuilder &s) {
    s.toJson(j);
    assert(j.contains("ClassName"));
}

void from_json(const nlohmann::json &j, ThreadLoopBuilder &s) {
    s.fromJson(j);
}

#endif //SETBENCH_THREAD_LOOP_BUILDER_H
