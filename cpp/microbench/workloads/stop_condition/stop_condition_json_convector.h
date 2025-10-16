//
// Created by Ravil Galiev on 31.07.2023.
//
#pragma once

#include "json/single_include/nlohmann/json.hpp"
#include "stop_condition.h"
#include "workloads/stop_condition/impls/timer.h"
#include "workloads/stop_condition/impls/operation_counter.h"
#include "errors.h"

StopCondition* getStopConditionFromJson(const nlohmann::json& j) {
    std::string className = j["ClassName"];
    StopCondition* stopCondition;
    if (className == "Timer") {
        stopCondition = new Timer();
    } else if (className == "OperationCounter") {
        stopCondition = new OperationCounter();
    } else {
        setbench_error("JSON PARSER: Unknown class name StopCondition -- " + className)
    }

    stopCondition->fromJson(j);
    return stopCondition;
}
