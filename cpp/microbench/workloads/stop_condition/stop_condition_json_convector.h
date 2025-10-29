//
// Created by Ravil Galiev on 31.07.2023.
//
#pragma once

#include "json/single_include/nlohmann/json.hpp"
#include "stop_condition.h"
#include "workloads/stop_condition/impls/timer.h"
#include "workloads/stop_condition/impls/operation_counter.h"
#include "errors.h"

namespace microbench::workload {

StopCondition* get_stop_condition_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    StopCondition* stop_condition;
    if (class_name == "Timer") {
        stop_condition = new Timer();
    } else if (class_name == "OperationCounter") {
        stop_condition = new OperationCounter();
    } else {
        setbench_error("JSON PARSER: Unknown class name StopCondition -- " + class_name)
    }

    stop_condition->from_json(j);
    return stop_condition;
}

}  // namespace microbench::workload
