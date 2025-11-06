//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_THREAD_LOOP_JSON_CONVECTOR_H
#define SETBENCH_THREAD_LOOP_JSON_CONVECTOR_H

#include "thread_loop_builder.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/thread_loops/impls/prefill_insert_thread_loop.h"
#include "workloads/thread_loops/impls/temporary_operations_thread_loop.h"
#include "errors.h"

ThreadLoopBuilder *getThreadLoopFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    ThreadLoopBuilder *threadLoopBuilder;

    if (className == "DefaultThreadLoopBuilder") {
        threadLoopBuilder = new DefaultThreadLoopBuilder();
    } else if (className == "TemporaryOperationsThreadLoopBuilder") {
        threadLoopBuilder = new TemporaryOperationsThreadLoopBuilder();
    } else if (className == "PrefillInsertThreadLoopBuilder") {
        threadLoopBuilder = new PrefillInsertThreadLoopBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name ThreadLoopBuilder -- " + className)
    }

    threadLoopBuilder->fromJson(j);
    return threadLoopBuilder;
}

#endif //SETBENCH_THREAD_LOOP_JSON_CONVECTOR_H
