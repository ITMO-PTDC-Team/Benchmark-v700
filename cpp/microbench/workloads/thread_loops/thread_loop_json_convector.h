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

std::shared_ptr<ThreadLoopBuilder> getThreadLoopFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    std::shared_ptr<ThreadLoopBuilder> threadLoopBuilder;

    if (className == "DefaultThreadLoopBuilder") {
        threadLoopBuilder = std::shared_ptr<DefaultThreadLoopBuilder>(new DefaultThreadLoopBuilder());
    } else if (className == "TemporaryOperationsThreadLoopBuilder") {
        threadLoopBuilder = std::shared_ptr<TemporaryOperationsThreadLoopBuilder>(new TemporaryOperationsThreadLoopBuilder());
    } else if (className == "PrefillInsertThreadLoopBuilder") {
        threadLoopBuilder = std::shared_ptr<PrefillInsertThreadLoopBuilder>(new PrefillInsertThreadLoopBuilder());
    } else {
        setbench_error("JSON PARSER: Unknown class name ThreadLoopBuilder -- " + className)
    }

    threadLoopBuilder->fromJson(j);
    return threadLoopBuilder;
}

#endif //SETBENCH_THREAD_LOOP_JSON_CONVECTOR_H
