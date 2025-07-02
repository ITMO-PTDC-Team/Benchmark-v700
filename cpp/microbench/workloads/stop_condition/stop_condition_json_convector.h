//
// Created by Ravil Galiev on 31.07.2023.
//

#ifndef SETBENCH_STOP_CONDITION_JSON_CONVECTOR_H
#define SETBENCH_STOP_CONDITION_JSON_CONVECTOR_H

#include "json/single_include/nlohmann/json.hpp"
#include "stop_condition.h"
#include "workloads/stop_condition/impls/timer.h"
#include "workloads/stop_condition/impls/operation_counter.h"
#include "errors.h"

class BaseStopConditionFactory {
    public:
        virtual ~BaseStopConditionFactory() = default;
        virtual StopCondition* create() = 0;
};
    
template <typename StopCondition>
class StopConditionFactory : public BaseStopConditionFactory {
    public:
    StopCondition *create() override {
        return new StopCondition();
    }
};

#define REGISTER_STOP_CONDITION_BUILDER(className) \
    map.insert({#className, std::make_unique<StopConditionFactory<className>>()})

inline static std::map<std::string, std::unique_ptr<BaseStopConditionFactory>> stopConditionFactoryMap = [] {
    std::map<std::string, std::unique_ptr<BaseStopConditionFactory>> map;
    REGISTER_STOP_CONDITION_BUILDER(Timer);
    REGISTER_STOP_CONDITION_BUILDER(OperationCounter);
    return map;
}();

StopCondition *getStopConditionFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    if (stopConditionFactoryMap.find(className) != stopConditionFactoryMap.end()) {
        StopCondition *stopCondition = stopConditionFactoryMap[className]->create();
        stopCondition->fromJson(j);
        return stopCondition;
    }
    setbench_error("JSON PARSER: Unknown class name StopCondition -- " + className);
}

#endif //SETBENCH_STOP_CONDITION_JSON_CONVECTOR_H
