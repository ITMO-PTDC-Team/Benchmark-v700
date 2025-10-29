//
// Created by Ravil Galiev on 25.07.2023.
//
#pragma once

#include "globals_extern.h"
#include "plaf.h"
#include "workloads/stop_condition/stop_condition.h"

class OperationCounter : public StopCondition {
    struct Counter {
        PAD;
        long long operCount;
        PAD;

        Counter()
            : operCount(0) {
        }

        explicit Counter(long long oper_count) {
            operCount = oper_count;
        }

        bool stop() {
            return --operCount < 0;
        }
    };

    PAD;
    Counter* counters_;
    PAD;
    size_t common_operation_limit_;
    PAD;

public:
    OperationCounter() {
    }

    explicit OperationCounter(size_t common_operation_limit)
        : common_operation_limit_(common_operation_limit) {
    }

    OperationCounter& set_common_operation_limit(size_t common_operation_limit) {
        OperationCounter::common_operation_limit_ = common_operation_limit;
        return *this;
    }

    void start(size_t num_threads) override {
        long long operation_limit = common_operation_limit_ / num_threads;
        long long remainder = common_operation_limit_ % num_threads;

        counters_ = new Counter[num_threads];

        for (int i = 0; i < num_threads; i++) {
            counters_[i].operCount = operation_limit + (--remainder >= 0 ? 1 : 0);
        }
    }

    void clean() override {
        delete[] counters_;
    }

    bool is_stopped(int id) override {
        return counters_[id].stop();
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "OperationCounter";
        j["commonOperationLimit"] = common_operation_limit_;
    }

    void from_json(const nlohmann::json& j) override {
        common_operation_limit_ = j["commonOperationLimit"];
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "OperationCounter", indents) +
               indented_title_with_data("commonOperationLimit", common_operation_limit_, indents);
    }

    ~OperationCounter() = default;
};
