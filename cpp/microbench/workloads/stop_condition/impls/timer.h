//
// Created by Ravil Galiev on 25.07.2023.
//

#ifndef SETBENCH_TIMER_H
#define SETBENCH_TIMER_H

#include <thread>
#include <string>
#include "plaf.h"
#include "workloads/stop_condition/stop_condition.h"
#include "json/single_include/nlohmann/json.hpp"

class Timer : public StopCondition {
    PAD;
    volatile bool stop;
    PAD;
    std::thread *stopThread;
    PAD;
    volatile bool isStarted;
    PAD;

public:
    void wait() {
        isStarted = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(workTime));
        stop = true;
    }

public:
    size_t workTime;

    Timer(size_t _workTime = 10000) : workTime(_workTime), stop(true) {}

    Timer &setWorkTime(size_t _workTime) {
        Timer::workTime = _workTime;
        return *this;
    }

    void start(size_t numThreads) override {
        stop = false;
        isStarted = false;
        stopThread = new std::thread(&Timer::wait, this);
        while (!isStarted);
    }

    void clean() override {
        stopThread->join();
        delete stopThread;
    }

    bool isStopped(int id) override {
        return stop;
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "Timer";
        j["workTime"] = workTime;
    }

    void fromJson(const nlohmann::json &j) override {
        workTime = j["workTime"];
    }

    ~Timer() override = default;

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Timer", indents)
               + indented_title_with_data("work time", workTime, indents);
    }
};

#endif //SETBENCH_TIMER_H
