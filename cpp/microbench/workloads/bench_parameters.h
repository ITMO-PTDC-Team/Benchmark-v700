//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_BENCH_PARAMETERS_H
#define SETBENCH_BENCH_PARAMETERS_H

#include "globals_extern.h"
#include "parameters.h"
#include "workloads/stop_condition/impls/operation_counter.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/data_maps/data_map_json_converter.h"

struct BenchParameters {
    size_t range;
    std::shared_ptr<DataMap> dataMap;

    Parameters* test;
    Parameters* prefill;
    Parameters* warmUp;

    BenchParameters() {
        range = 2048;
        //        test = nullptr;
        //        prefill = nullptr;
        //        warmUp = nullptr;
        test = new Parameters();
        prefill = new Parameters();
        warmUp = new Parameters();
    }

    BenchParameters(const BenchParameters& p) = default;

    BenchParameters& createDefaultPrefill(size_t threadNum) {
        prefill = (new Parameters())
                      ->setStopCondition(new OperationCounter(range / 2))
                      ->addThreadLoopBuilder(new PrefillInsertThreadLoopBuilder(), threadNum);
        return *this;
    }

    BenchParameters& createDefaultPrefill() {
        prefill = new Parameters();
        prefill->stopCondition = new OperationCounter(range / 2);
        prefill->addThreadLoopBuilder(new PrefillInsertThreadLoopBuilder(), 1);
        return createDefaultPrefill(1);
    }

    BenchParameters& setRange(size_t _range) {
        range = _range;
        return *this;
    }

    BenchParameters& setDataMap(DataMap* _dataMap) {
        dataMap.reset(_dataMap);
        return *this;
    }

    BenchParameters& setTest(Parameters* _test) {
        test = _test;
        return *this;
    }

    BenchParameters& setPrefill(Parameters* _prefill) {
        prefill = _prefill;
        return *this;
    }

    BenchParameters& setWarmUp(Parameters* _warmUp) {
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
        dataMap->init(range);
        initIndexMapBuilders(range);

        prefill->init(range);
        warmUp->init(range);
        test->init(range);
    }

    std::string toString(size_t indents = 1) {
        return indented_title_with_data("Range", range, indents) +
               dataMap->toString(indents + 1) + 
               (prefill->getNumThreads() == 0
                    ? toStringStage("without prefill")
                    : toStringStage("prefill parameters") + prefill->toString(indents + 1)) +
               (warmUp->getNumThreads() == 0
                    ? toStringStage("without warmUp")
                    : toStringStage("warmUp parameters") + warmUp->toString(indents + 1)) +
               toStringStage("test parameters") + test->toString(indents + 1);
    }

    ~BenchParameters() {
        delete test;
        delete prefill;
        delete warmUp;
        deleteIndexMapBuilders();
    }
};

void to_json(nlohmann::json& json, const BenchParameters& s) {
    json["range"] = s.range;
    s.dataMap->toJson(json["dataMap"]);

    json["test"] = *s.test;
    json["prefill"] = *s.prefill;
    json["warmUp"] = *s.warmUp;
}

void from_json(const nlohmann::json& json, BenchParameters& s) {
    s.range = json["range"];
    s.dataMap.reset(getDataMapFromJson(json["dataMap"]));

    s.test = new Parameters(json["test"]);
    s.prefill = new Parameters(json["prefill"]);
    s.warmUp = new Parameters(json["warmUp"]);
}

#endif  // SETBENCH_BENCH_PARAMETERS_H
