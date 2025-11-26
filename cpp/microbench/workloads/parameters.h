//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "workloads/stop_condition/stop_condition.h"
#include "workloads/stop_condition/impls/timer.h"
#include "workloads/thread_loops/thread_loop_builder.h"
#include "workloads/thread_loops/default/impls/default_thread_loop.h"
#include "globals_t.h"
#include "workloads/thread_loops/thread_loop.h"
#include "workloads/stop_condition/stop_condition_json_convector.h"
#include "workloads/thread_loops/thread_loop_json_convector.h"
#include "binding.h"

namespace microbench::workload {

struct ThreadLoopSettings {
    ThreadLoopBuilder* threadLoopBuilder;
    size_t quantity;
    std::string pinPattern;

    explicit ThreadLoopSettings(const nlohmann::json& j) {
        quantity = j["quantity"];
        if (j.contains("pin")) {
            pinPattern = j["pin"].get<std::string>();
        } else {
            pinPattern = "";
        }
        threadLoopBuilder = get_thread_loop_from_json(j["threadLoopBuilder"]);
    }

    ThreadLoopSettings* set_thread_loop_builder(ThreadLoopBuilder* thread_loop_builder) {
        threadLoopBuilder = thread_loop_builder;
        return this;
    }

    ThreadLoopSettings* set_quantity(size_t quantity) {
        quantity = quantity;
        return this;
    }

    ThreadLoopSettings* set_pin(const std::string& pin_pattern) {
        pinPattern = pin_pattern;
        return this;
    }

    ThreadLoopSettings() = default;

    explicit ThreadLoopSettings(ThreadLoopBuilder* thread_loop_builder, size_t quantity = 1,
                                const std::string& pin_pattern = "")
        : threadLoopBuilder(thread_loop_builder),
          quantity(quantity),
          pinPattern(pin_pattern) {
    }

    ~ThreadLoopSettings() {
        delete threadLoopBuilder;
    }
};

void to_json(nlohmann::json& j, const ThreadLoopSettings& s) {
    j["quantity"] = s.quantity;
    j["threadLoopBuilder"] = *s.threadLoopBuilder;
    if (!s.pinPattern.empty()) {
        j["pin"] = s.pinPattern;
    }
}

void from_json(const nlohmann::json& j, ThreadLoopSettings& s) {
    s.quantity = j["quantity"];
    s.pinPattern = "";
    if (j.contains("pin")) {
        s.pinPattern = j["pin"].get<std::string>();
    } else {
        s.pinPattern = "~" + std::to_string(s.quantity);
    }
    s.threadLoopBuilder = get_thread_loop_from_json(j["threadLoopBuilder"]);
}

class Parameters {
    size_t num_threads_;
    std::vector<int> pin_;

public:
    static void parse_binding(std::string& pin_pattern, std::vector<int>& result_pin) {
        std::istringstream iss(pin_pattern);
        char c;
        int num;

        while (iss >> c) {
            if (c == '~') {
                char next = iss.peek();
                if (next == '.') {
                    result_pin.push_back(-1);
                } else if (iss >> num) {
                    result_pin.insert(result_pin.end(), num, -1);
                }
            } else if (std::isdigit(c) != 0) {
                iss.putback(c);
                if (iss >> num) {
                    char next = iss.peek();
                    if (next == '-') {
                        iss >> c;
                        int end;
                        if (iss >> end) {
                            for (int i = num; i <= end; ++i) {
                                result_pin.push_back(i);
                            }
                        }
                    } else {
                        result_pin.push_back(num);
                    }
                }
            }
        }
    }

    StopCondition* stopCondition;

    std::vector<ThreadLoopSettings*> threadLoopBuilders;

    //    Parameters() : numThreads(0), stopCondition(nullptr) {}
    Parameters()
        : num_threads_(0),
          stopCondition(new Timer(5000)) {
    }

    Parameters(const Parameters& p) = default;

    size_t get_num_threads() const {
        return num_threads_;
    }

    const std::vector<int>& get_pin() const {
        return pin_;
    }

    Parameters* set_stop_condition(StopCondition* stop_condition) {
        stopCondition = stop_condition;
        return this;
    }

    Parameters* set_thread_loop_builders(
        const std::vector<ThreadLoopSettings*>& thread_loop_builders) {
        Parameters::threadLoopBuilders = thread_loop_builders;
        return this;
    }

    Parameters* add_thread_loop_builder(ThreadLoopSettings* thread_loop_settings) {
        threadLoopBuilders.push_back(thread_loop_settings);
        num_threads_ += thread_loop_settings->quantity;
        if ((thread_loop_settings != nullptr) && !thread_loop_settings->pinPattern.empty()) {
            std::vector<int> cur_pin;
            parse_binding(thread_loop_settings->pinPattern, cur_pin);
            for (size_t i = 0; i < thread_loop_settings->quantity; ++i) {
                pin_.push_back(cur_pin[i % cur_pin.size()]);
            }
        } else {
            pin_.resize(num_threads_, -1);
        }
        // assert(numThreads == pin.size());
        return this;
    }

    Parameters* add_thread_loop_builder(ThreadLoopBuilder* thread_loop_builder, size_t quantity = 1,
                                        const std::string& pin_pattern = "") {
        return add_thread_loop_builder(
            new ThreadLoopSettings(thread_loop_builder, quantity, pin_pattern));
    }

    Parameters* init(int range) {
        if (stopCondition == nullptr) {
            stopCondition = new Timer(5000);
        }

        for (ThreadLoopSettings* thread_loop_settings : threadLoopBuilders) {
            thread_loop_settings->threadLoopBuilder->init(range);
        }
        return this;
    }

    ThreadLoop** get_workload(globals_t* g, Random64* rngs) const {
        ThreadLoop** workload = new ThreadLoop*[this->num_threads_];
        for (size_t thread_id = 0, i = 0, cur_quantity = 0; thread_id < this->num_threads_;
             ++thread_id, ++cur_quantity) {
            if (cur_quantity >= threadLoopBuilders[i]->quantity) {
                cur_quantity = 0;
                ++i;
            }

            workload[thread_id] = threadLoopBuilders[i]->threadLoopBuilder->build(
                g, rngs[thread_id], thread_id, stopCondition);
        }
        return workload;
    }

    void to_json(nlohmann::json& j) const {
        j["numThreads"] = num_threads_;
        j["stopCondition"] = *stopCondition;
        for (ThreadLoopSettings* tls : threadLoopBuilders) {
            j["threadLoopBuilders"].push_back(*tls);
        }
    }

    void from_json(const nlohmann::json& j) {
        stopCondition = get_stop_condition_from_json(j["stopCondition"]);

        if (j.contains("threadLoopBuilders")) {
            for (const auto& i : j["threadLoopBuilders"]) {
                add_thread_loop_builder(new ThreadLoopSettings(i));
            }
        }
    }

    std::string to_string(size_t indents = 1) {
        std::string result =
            indented_title_with_data("number of threads", num_threads_, indents) +
            indented_title("stop condition", indents) + stopCondition->to_string(indents + 1) +
            indented_title_with_data("number of workloads", threadLoopBuilders.size(), indents);

        std::string pin_string = std::to_string(pin_[0]);
        for (size_t i = 1; i < num_threads_; ++i) {
            pin_string += "," + std::to_string(pin_[i]);
        }

        result += indented_title_with_str_data("all pins", pin_string, indents) +
                  indented_title("thread loops", indents);

        for (auto tls : threadLoopBuilders) {
            result += indented_title_with_data("quantity", tls->quantity, indents + 1);

            if (!tls->pinPattern.empty()) {
                result += indented_title_with_str_data("pin", tls->pinPattern, indents + 1);
            }

            result += tls->threadLoopBuilder->to_string(indents + 2);
        }
        return result;
    }

    ~Parameters() {
        delete stopCondition;
        for (ThreadLoopSettings* tls : threadLoopBuilders) {
            delete tls;
        }
    }
};

void to_json(nlohmann::json& json, const Parameters& s) {
    s.to_json(json);
}

void from_json(const nlohmann::json& j, Parameters& s) {
    s.from_json(j);
}

}  // namespace microbench::workload
