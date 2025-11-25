//
// Created by Ravil Galiev on 16.08.2023.
//
#pragma once

#include "globals_extern.h"
#include "json/single_include/nlohmann/json.hpp"

namespace microbench::workload {

struct RatioThreadLoopParameters {
    double INS_RATIO;
    double REM_RATIO;
    double RQ_RATIO;

    #if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)
    double PUSH_RATIO;
    double POP_RATIO;
    #endif

    RatioThreadLoopParameters()
        : RatioThreadLoopParameters(0, 0, 0) {
    }

    RatioThreadLoopParameters(double ins_ratio, double rem_ratio, double rq_ratio)
        : INS_RATIO(ins_ratio),
          REM_RATIO(rem_ratio),
          RQ_RATIO(rq_ratio) {
    }

    RatioThreadLoopParameters(const RatioThreadLoopParameters& ratio) = default;

    RatioThreadLoopParameters* set_ins_ratio(double ins_ratio) {
        INS_RATIO = ins_ratio;
        return this;
    }

    RatioThreadLoopParameters* set_rem_ratio(double rem_ratio) {
        REM_RATIO = rem_ratio;
        return this;
    }

    RatioThreadLoopParameters* set_rq_ratio(double rq_ratio) {
        RQ_RATIO = rq_ratio;
        return this;
    }

    #if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)

    RatioThreadLoopParameters* set_push_ratio(double push_ratio) {
        PUSH_RATIO = push_ratio;
        return this;
    }

    RatioThreadLoopParameters* set_pop_ratio(double pop_ratio) {
        POP_RATIO = pop_ratio;
        return this;
    }

    #endif

    std::string to_string(const size_t indents = 1) {
        return indented_title_with_data("INS_RATIO", INS_RATIO, indents) +
               indented_title_with_data("REM_RATIO", REM_RATIO, indents) +
               indented_title_with_data("RQ_RATIO", RQ_RATIO, indents)
               #if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)
               + indented_title_with_data("PUSH_RATIO", PUSH_RATIO, indents) 
               + indented_title_with_data("POP_RATIO", POP_RATIO, indents);
               #else
               ;
               #endif
    }
};

void to_json(nlohmann::json& j, const RatioThreadLoopParameters& s) {
    j["insertRatio"] = s.INS_RATIO;
    j["removeRatio"] = s.REM_RATIO;
    j["rqRatio"] = s.RQ_RATIO;
    #if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)
    j["popRatio"] = s.POP_RATIO;
    j["pushRatio"] = s.PUSH_RATIO;
    #endif
}

void from_json(const nlohmann::json& j, RatioThreadLoopParameters& s) {
    s.INS_RATIO = j["insertRatio"];
    s.REM_RATIO = j["removeRatio"];
    if (j.contains("rqRatio")) {
        s.RQ_RATIO = j["rqRatio"];
    }
    #if defined(USE_STACK_OPERATIONS) || defined(USE_QUEUE_OPERATIONS)
    s.PUSH_RATIO = j["pushRatio"];
    s.POP_RATIO = j["popRatio"];
    #endif
}

}  // namespace microbench::workload
