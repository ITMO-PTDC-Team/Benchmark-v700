//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_PARAMETERS_H
#define SETBENCH_PARAMETERS_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "workloads/stop_condition/stop_condition.h"
#include "workloads/stop_condition/impls/timer.h"
#include "workloads/thread_loops/thread_loop_builder.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "globals_t.h"
#include "workloads/thread_loops/thread_loop.h"
#include "workloads/stop_condition/stop_condition_json_convector.h"
#include "workloads/thread_loops/thread_loop_json_convector.h"
#include "binding.h"

struct ThreadLoopSettings {
    ThreadLoopBuilder *threadLoopBuilder;
    size_t quantity;
    std::string pinPattern;

    ThreadLoopSettings(const nlohmann::json &j) {
        quantity = j["quantity"];
        if (j.contains("pin")) {
            pinPattern = j["pin"].get<std::string>();
        } else {
            pinPattern = "";
        }
        threadLoopBuilder = getThreadLoopFromJson(j["threadLoopBuilder"]);
    }

    ThreadLoopSettings *setThreadLoopBuilder(ThreadLoopBuilder *_threadLoopBuilder) {
        threadLoopBuilder = _threadLoopBuilder;
        return this;
    }

    ThreadLoopSettings *setQuantity(size_t _quantity) {
        quantity = _quantity;
        return this;
    }

    ThreadLoopSettings *setPin(const std::string& _pinPattern) {
        pinPattern = _pinPattern;
        return this;
    }

    ThreadLoopSettings() {}

    ThreadLoopSettings(ThreadLoopBuilder *threadLoopBuilder, size_t quantity = 1, const std::string& _pinPattern = "")
            : threadLoopBuilder(threadLoopBuilder), quantity(quantity), pinPattern(_pinPattern) {}

    ~ThreadLoopSettings() {
        delete threadLoopBuilder;
    }
};


void to_json(nlohmann::json &j, const ThreadLoopSettings &s) {
    j["quantity"] = s.quantity;
    j["threadLoopBuilder"] = *s.threadLoopBuilder;
    if (!s.pinPattern.empty()) {
        j["pin"] = s.pinPattern;
    }
}

void from_json(const nlohmann::json &j, ThreadLoopSettings &s) {
    s.quantity = j["quantity"];
    s.pinPattern = "";
    if (j.contains("pin")) {
        s.pinPattern = j["pin"].get<std::string>();
    } else {
        s.pinPattern = "~" + std::to_string(s.quantity);
    }
    s.threadLoopBuilder = getThreadLoopFromJson(j["threadLoopBuilder"]);
}

class Parameters {
    size_t numThreads;
    std::vector<int> pin;

public:
    static void parseBinding(std::string& pinPattern, std::vector<int> & resultPin) {
        std::istringstream iss(pinPattern);
        char c;
        int num;
        
        while (iss >> c) {
            if (c == '~') {  
                char next = iss.peek();
                if (next == '.') {
                    resultPin.push_back(-1);
                } else if (iss >> num) {
                    resultPin.insert(resultPin.end(), num, -1);
                }
            } else if (isdigit(c)) {  
                iss.putback(c);
                if (iss >> num) {
                    char next = iss.peek();
                    if (next == '-') {  
                        iss >> c;  
                        int end;
                        if (iss >> end) {
                            for (int i = num; i <= end; ++i) {
                                resultPin.push_back(i);
                            }
                        }
                    } else {  
                        resultPin.push_back(num);
                    }
                }
            }
        }
    }

    StopCondition *stopCondition;

    std::vector<ThreadLoopSettings *> threadLoopBuilders;

//    Parameters() : numThreads(0), stopCondition(nullptr) {}
    Parameters() : numThreads(0), stopCondition(new Timer(5000)) {}

    Parameters(const Parameters &p) = default;

    size_t getNumThreads() const {
        return numThreads;
    }

    const std::vector<int> &getPin() const {
        return pin;
    }

    Parameters *setStopCondition(StopCondition *_stopCondition) {
        stopCondition = _stopCondition;
        return this;
    }

    Parameters *setThreadLoopBuilders(const std::vector<ThreadLoopSettings *> &_threadLoopBuilders) {
        Parameters::threadLoopBuilders = _threadLoopBuilders;
        return this;
    }

    Parameters *addThreadLoopBuilder(ThreadLoopSettings *_threadLoopSettings) {
        threadLoopBuilders.push_back(_threadLoopSettings);
        numThreads += _threadLoopSettings->quantity;
        if (_threadLoopSettings && !_threadLoopSettings->pinPattern.empty()) {
            std::vector<int> curPin;
            parseBinding(_threadLoopSettings->pinPattern, curPin);
            for (size_t i = 0; i < _threadLoopSettings->quantity; ++i) {
                pin.push_back(curPin[i % curPin.size()]);
            }
        } else {
            pin.resize(numThreads, -1);
        }
        assert(numThreads == pin.size());
        return this;
    }

    Parameters *addThreadLoopBuilder(ThreadLoopBuilder *_threadLoopBuilder,
                                     size_t quantity = 1,
                                     const std::string& _pinPattern = "") {
        return addThreadLoopBuilder(new ThreadLoopSettings(_threadLoopBuilder, quantity, _pinPattern));
    }

    Parameters *init(int range) {
        if (stopCondition == nullptr) {
            stopCondition = new Timer(5000);
        }

        for (ThreadLoopSettings *threadLoopSettings: threadLoopBuilders) {
            threadLoopSettings->threadLoopBuilder->init(range);
        }
        return this;
    }

    ThreadLoop **getWorkload(globals_t *_g, Random64 *_rngs) const {
        ThreadLoop **workload = new ThreadLoop *[this->numThreads];
        for (size_t threadId = 0, i = 0, curQuantity = 0; threadId < this->numThreads; ++threadId, ++curQuantity) {
            if (curQuantity >= threadLoopBuilders[i]->quantity) {
                curQuantity = 0;
                ++i;
            }

            workload[threadId] = threadLoopBuilders[i]->threadLoopBuilder
                    ->build(_g, _rngs[threadId], threadId, stopCondition);
        }
        return workload;
    }

    void toJson(nlohmann::json &j) const {
        j["numThreads"] = numThreads;
        j["stopCondition"] = *stopCondition;
        for (ThreadLoopSettings *tls: threadLoopBuilders) {
            j["threadLoopBuilders"].push_back(*tls);
        }
    }

    void fromJson(const nlohmann::json &j) {
        stopCondition = getStopConditionFromJson(j["stopCondition"]);

        if (j.contains("threadLoopBuilders")) {
            for (const auto &i: j["threadLoopBuilders"]) {
                addThreadLoopBuilder(new ThreadLoopSettings(i));
            }
        }
    }

    std::string toString(size_t indents = 1) {
        std::string result = indented_title_with_data("number of threads", numThreads, indents)
                             + indented_title("stop condition", indents)
                             + stopCondition->toString(indents + 1)
                             + indented_title_with_data("number of workloads", threadLoopBuilders.size(), indents);


        std::string pin_string = std::to_string(pin[0]);
        for (size_t i = 1; i < numThreads; ++i) {
            pin_string += "," + std::to_string(pin[i]);
        }

        result += indented_title_with_str_data("all pins", pin_string, indents)
                  + indented_title("thread loops", indents);


        for (auto tls: threadLoopBuilders) {
            result += indented_title_with_data("quantity", tls->quantity, indents + 1);

            if (!tls->pinPattern.empty()) {
                result += indented_title_with_str_data("pin", tls->pinPattern, indents + 1);
            }

            result += tls->threadLoopBuilder->toString(indents + 2);
        }
        return result;
    }

    ~Parameters() {
        delete stopCondition;
        for (ThreadLoopSettings *tls: threadLoopBuilders) {
            delete tls;
        }
    }

};

void to_json(nlohmann::json &json, const Parameters &s) {
    s.toJson(json);
}

void from_json(const nlohmann::json &j, Parameters &s) {
    s.fromJson(j);
}

#endif //SETBENCH_PARAMETERS_H
