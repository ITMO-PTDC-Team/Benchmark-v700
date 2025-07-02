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

class BaseThreadLoopBuilderFactory {
    public:
        virtual ~BaseThreadLoopBuilderFactory() = default;
        virtual ThreadLoopBuilder* create() = 0;
};
    
template <typename ThreadLoopBuilder>
class ThreadLoopBuilderFactory : public BaseThreadLoopBuilderFactory {
    public:
    ThreadLoopBuilder *create() override {
        return new ThreadLoopBuilder();
    }
};

#define REGISTER_THREAD_LOOP_BUILDER(className) \
    map.insert({#className, std::make_unique<ThreadLoopBuilderFactory<className>>()})

inline static std::map<std::string, std::unique_ptr<BaseThreadLoopBuilderFactory>> threadLoopFactoryMap = [] {
    std::map<std::string, std::unique_ptr<BaseThreadLoopBuilderFactory>> map;
    REGISTER_THREAD_LOOP_BUILDER(DefaultThreadLoopBuilder);
    REGISTER_THREAD_LOOP_BUILDER(TemporaryOperationsThreadLoopBuilder);
    REGISTER_THREAD_LOOP_BUILDER(PrefillInsertThreadLoopBuilder);
    return map;
}();

ThreadLoopBuilder *getThreadLoopFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    if (threadLoopFactoryMap.find(className) != threadLoopFactoryMap.end()) {
        ThreadLoopBuilder *threadLoopBuilder = threadLoopFactoryMap[className]->create();
        threadLoopBuilder->fromJson(j);
        return threadLoopBuilder;
    }
    setbench_error("JSON PARSER: Unknown class name ThreadLoopBuilder -- " + className)
}

#endif //SETBENCH_THREAD_LOOP_JSON_CONVECTOR_H
