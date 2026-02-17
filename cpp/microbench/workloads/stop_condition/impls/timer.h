//
// Created by Ravil Galiev on 25.07.2023.
//
#pragma once

#include <thread>
#include <string>
#include "globals_extern.h"
#include "plaf.h"
#include "workloads/stop_condition/stop_condition.h"
#include "nlohmann/json.hpp"

namespace microbench::workload {

class Timer : public StopCondition {
    PAD;
    volatile bool stop_;
    PAD;
    std::thread* stop_thread_;
    PAD;
    volatile bool is_started_;
    PAD;

public:
    void wait() {
        is_started_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(workTime));
        stop_ = true;
    }

public:
    size_t workTime;

    explicit Timer(size_t work_time = 10000)
        : workTime(work_time),
          stop_(true) {
    }

    Timer& set_work_time(size_t work_time) {
        Timer::workTime = work_time;
        return *this;
    }

    void start(size_t num_threads) override {
        stop_ = false;
        is_started_ = false;
        stop_thread_ = new std::thread(&Timer::wait, this);
        while (!is_started_) {
        };
    }

    void clean() override {
        stop_thread_->join();
        delete stop_thread_;
    }

    bool is_stopped(int id) override {
        return stop_;
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "Timer";
        j["workTime"] = workTime;
    }

    void from_json(const nlohmann::json& j) override {
        workTime = j["workTime"];
    }

    ~Timer() override = default;

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Timer", indents) +
               indented_title_with_data("work time", workTime, indents);
    }
};

}  // namespace microbench::workload
