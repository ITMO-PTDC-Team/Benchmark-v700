//
// Created by Ravil Galiev on 25.07.2023.
//

#ifndef SETBENCH_OPERATION_COUNTER_H
#define SETBENCH_OPERATION_COUNTER_H

#include "plaf.h"
#include "workloads/stop_condition/stop_condition.h"

class OperationCounter : public StopCondition {
    struct Counter {
        PAD;
        long long operCount;
        PAD;

        Counter() : operCount(0) {}

        Counter(long long _operCount) {
            operCount = _operCount;
        }

        bool stop() {
            return --operCount < 0;
        }
    };

    PAD;
    Counter *counters;
    PAD;
    size_t commonOperationLimit;
    PAD;

public:

    OperationCounter() {}

    OperationCounter(size_t _commonOperationLimit) : commonOperationLimit(_commonOperationLimit) {}

    OperationCounter &setCommonOperationLimit(size_t _commonOperationLimit) {
        OperationCounter::commonOperationLimit = _commonOperationLimit;
        return *this;
    }

    void start(size_t numThreads) override {
        long long operationLimit = commonOperationLimit / numThreads;
        long long remainder = commonOperationLimit % numThreads;

        counters = new Counter[numThreads];

        for (int i = 0; i < numThreads; i++) {
            counters[i].operCount = operationLimit + (--remainder >= 0 ? 1 : 0);
        }
    }

    void clean() override {
        delete[] counters;
    }

    bool isStopped(int id) override {
        return counters[id].stop();
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "OperationCounter";
        j["commonOperationLimit"] = commonOperationLimit;
    }

    void fromJson(const nlohmann::json &j) override {
        commonOperationLimit = j["commonOperationLimit"];
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "OperationCounter", indents)
               + indented_title_with_data("commonOperationLimit", commonOperationLimit, indents);
    }

    ~OperationCounter() = default;

};

#endif //SETBENCH_OPERATION_COUNTER_H
