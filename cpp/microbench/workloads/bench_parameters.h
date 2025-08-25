//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_BENCH_PARAMETERS_H
#define SETBENCH_BENCH_PARAMETERS_H

#include "globals_extern.h"
#include "parameters.h"
#include "workloads/stop_condition/impls/operation_counter.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/thread_loops/impls/prefill_insert_thread_loop.h"

struct BenchParameters {
    size_t range;

    std::shared_ptr<Parameters> prefill;
    std::shared_ptr<Parameters> warmUp;
    std::shared_ptr<Parameters> test;

    BenchParameters() {
        range = 2048;
        //        test = nullptr;
        //        prefill = nullptr;
        //        warmUp = nullptr;
        test = std::make_shared<Parameters>();
        prefill = std::make_shared<Parameters>();
        warmUp = std::make_shared<Parameters>();
    }

    BenchParameters(const BenchParameters& p) = default;

    BenchParameters& createDefaultPrefill(size_t threadNum) {
        prefill = std::shared_ptr<Parameters>((new Parameters())
                      ->setStopCondition(std::shared_ptr<OperationCounter>(new OperationCounter(range / 2)))
                      ->addThreadLoopBuilder(std::shared_ptr<PrefillInsertThreadLoopBuilder>(new PrefillInsertThreadLoopBuilder()), threadNum));
        return *this;
    }

    BenchParameters& createDefaultPrefill() {
        prefill = std::shared_ptr<Parameters>(new Parameters());
        prefill->stopCondition = std::shared_ptr<OperationCounter>(new OperationCounter(range / 2));
        prefill->addThreadLoopBuilder(std::shared_ptr<PrefillInsertThreadLoopBuilder>(new PrefillInsertThreadLoopBuilder()), 1);
        return createDefaultPrefill(1);
    }

    BenchParameters& setRange(size_t _range) {
        range = _range;
        return *this;
    }

    BenchParameters& setTest(std::shared_ptr<Parameters> _test) {
        test = _test;
        return *this;
    }

    BenchParameters& setPrefill(std::shared_ptr<Parameters> _prefill) {
        prefill = _prefill;
        return *this;
    }

    BenchParameters& setWarmUp(std::shared_ptr<Parameters> _warmUp) {
        warmUp = _warmUp;
        return *this;
    }

    size_t getTotalThreads() {
        return prefill->getNumThreads() + warmUp->getNumThreads() + test->getNumThreads();
    }

    size_t getMaxThreads() {
        return std::max(prefill->getNumThreads(),
                        std::max(warmUp->getNumThreads(), test->getNumThreads()));
    }

    void init() {
        //        if (test == nullptr) {
        //            test = new Parameters();
        //        }
        //        if (prefill == nullptr) {
        //            prefill = new Parameters();
        //        }
        //        if (warmUp == nullptr) {
        //            warmUp = new Parameters();
        //        }
        initDataMapBuilders(range);
        prefill->init(range);
        warmUp->init(range);
        test->init(range);
    }

    std::string toString(size_t indents = 1) {
        return indented_title_with_data("Range", range, indents) +
               (prefill->getNumThreads() == 0
                    ? toStringStage("without prefill")
                    : toStringStage("prefill parameters") + prefill->toString(indents + 1)) +
               (warmUp->getNumThreads() == 0
                    ? toStringStage("without warmUp")
                    : toStringStage("warmUp parameters") + warmUp->toString(indents + 1)) +
               toStringStage("test parameters") + test->toString(indents + 1);
    }

    ~BenchParameters() {
        deleteDataMapBuilders();
    }
};

void to_json(nlohmann::json& json, const BenchParameters& s) {
    json["range"] = s.range;
    json["test"] = *s.test;
    json["prefill"] = *s.prefill;
    json["warmUp"] = *s.warmUp;
}

void from_json(const nlohmann::json& json, BenchParameters& s) {
    s.range = json["range"];
    s.test = std::make_shared<Parameters>(json["test"]);
    s.prefill = std::make_shared<Parameters>(json["prefill"]);
    s.warmUp = std::make_shared<Parameters>(json["warmUp"]);
}

#endif  // SETBENCH_BENCH_PARAMETERS_H
