//
// Created by Ravil Galiev on 16.08.2023.
//

#ifndef SETBENCH_RATIO_THREAD_LOOP_PARAMETERS_H
#define SETBENCH_RATIO_THREAD_LOOP_PARAMETERS_H

#include "globals_extern.h"
#include "json/single_include/nlohmann/json.hpp"

struct RatioThreadLoopParameters {
    double INS_RATIO;
    double REM_RATIO;
    double RQ_RATIO;

    RatioThreadLoopParameters() : RatioThreadLoopParameters(0, 0, 0) {}

    RatioThreadLoopParameters(double insRatio, double remRatio, double rqRatio)
            : INS_RATIO(insRatio),
              REM_RATIO(remRatio),
              RQ_RATIO(rqRatio) {}

    RatioThreadLoopParameters(const RatioThreadLoopParameters &ratio) = default;

    RatioThreadLoopParameters *setInsRatio(double insRatio) {
        INS_RATIO = insRatio;
        return this;
    }

    RatioThreadLoopParameters *setRemRatio(double remRatio) {
        REM_RATIO = remRatio;
        return this;
    }

    RatioThreadLoopParameters *setRqRatio(double rqRatio) {
        RQ_RATIO = rqRatio;
        return this;
    }

    std::string toString(const size_t indents = 1) {
        return indented_title_with_data("INS_RATIO", INS_RATIO, indents)
               + indented_title_with_data("REM_RATIO", REM_RATIO, indents)
               + indented_title_with_data("RQ_RATIO", RQ_RATIO, indents);
    }
};

void to_json(nlohmann::json &j, const RatioThreadLoopParameters &s) {
    j["insertRatio"] = s.INS_RATIO;
    j["removeRatio"] = s.REM_RATIO;
    j["rqRatio"] = s.RQ_RATIO;
}

void from_json(const nlohmann::json &j, RatioThreadLoopParameters &s) {
    s.INS_RATIO = j["insertRatio"];
    s.REM_RATIO = j["removeRatio"];
    if (j.contains("rqRatio")) {
        s.RQ_RATIO = j["rqRatio"];
    }
}

#endif //SETBENCH_RATIO_THREAD_LOOP_PARAMETERS_H
