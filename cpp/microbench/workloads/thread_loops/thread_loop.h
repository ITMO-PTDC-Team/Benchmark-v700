//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "workloads/stop_condition/stop_condition.h"
#include "globals_t.h"

namespace microbench::workload {

using K = int64_t;

class ThreadLoop {
protected:
    K garbage = 0;
    VALUE_TYPE NO_VALUE;

public:
    size_t threadId;
    globals_t* g;
    StopCondition* stopCondition;

    ThreadLoop() = default;

    virtual void run();

    virtual void step() = 0;
};

}  // namespace microbench::workload