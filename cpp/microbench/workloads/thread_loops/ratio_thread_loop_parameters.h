//
// Created by Ravil Galiev on 16.08.2023.
//
#pragma once

#include "globals_extern.h"
#include "nlohmann/json.hpp"

namespace microbench::workload {

struct RatioThreadLoopParameters {
    double INS_RATIO;
    double REM_RATIO;
    double RQ_RATIO;

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

    std::string to_string(const size_t indents = 1) {
        return indented_title_with_data("INS_RATIO", INS_RATIO, indents) +
               indented_title_with_data("REM_RATIO", REM_RATIO, indents) +
               indented_title_with_data("RQ_RATIO", RQ_RATIO, indents);
    }
};

void to_json(nlohmann::json& j, const RatioThreadLoopParameters& s) {
    j["insertRatio"] = s.INS_RATIO;
    j["removeRatio"] = s.REM_RATIO;
    j["rqRatio"] = s.RQ_RATIO;
}

void from_json(const nlohmann::json& j, RatioThreadLoopParameters& s) {
    s.INS_RATIO = j["insertRatio"];
    s.REM_RATIO = j["removeRatio"];
    if (j.contains("rqRatio")) {
        s.RQ_RATIO = j["rqRatio"];
    }
}

}  // namespace microbench::workload
