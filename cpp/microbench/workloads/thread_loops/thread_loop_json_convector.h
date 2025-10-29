//
// Created by Ravil Galiev on 27.07.2023.
//
#pragma once

#include "thread_loop_builder.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/thread_loops/impls/prefill_insert_thread_loop.h"
#include "workloads/thread_loops/impls/temporary_operations_thread_loop.h"
#include "errors.h"

ThreadLoopBuilder* get_thread_loop_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    ThreadLoopBuilder* thread_loop_builder;

    if (class_name == "DefaultThreadLoopBuilder") {
        thread_loop_builder = new DefaultThreadLoopBuilder();
    } else if (class_name == "TemporaryOperationsThreadLoopBuilder") {
        thread_loop_builder = new TemporaryOperationsThreadLoopBuilder();
    } else if (class_name == "PrefillInsertThreadLoopBuilder") {
        thread_loop_builder = new PrefillInsertThreadLoopBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name ThreadLoopBuilder -- " + class_name)
    }

    thread_loop_builder->from_json(j);
    return thread_loop_builder;
}
