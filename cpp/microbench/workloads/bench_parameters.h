//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include "globals_extern.h"
#include "parameters.h"
#include "workloads/stop_condition/impls/operation_counter.h"

namespace microbench::workload {

struct BenchParameters {
    size_t range;

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

    BenchParameters& create_default_prefill(size_t thread_num) {
        prefill = (new Parameters())
                      ->set_stop_condition(new OperationCounter(range / 2))
                      ->add_thread_loop_builder(new PrefillInsertThreadLoopBuilder(), thread_num);
        return *this;
    }

    BenchParameters& create_default_prefill() {
        prefill = new Parameters();
        prefill->stopCondition = new OperationCounter(range / 2);
        prefill->add_thread_loop_builder(new PrefillInsertThreadLoopBuilder(), 1);
        return create_default_prefill(1);
    }

    BenchParameters& set_range(size_t range) {
        range = range;
        return *this;
    }

    BenchParameters& set_test(Parameters* test) {
        test = test;
        return *this;
    }

    BenchParameters& set_prefill(Parameters* prefill) {
        prefill = prefill;
        return *this;
    }

    BenchParameters& set_warm_up(Parameters* warm_up) {
        warmUp = warm_up;
        return *this;
    }

    size_t get_total_threads() {
        return prefill->get_num_threads() + warmUp->get_num_threads() + test->get_num_threads();
    }

    size_t get_max_threads() {
        return std::max(prefill->get_num_threads(),
                        std::max(warmUp->get_num_threads(), test->get_num_threads()));
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
        init_data_map_builders(range);
        prefill->init(range);
        warmUp->init(range);
        test->init(range);
    }

    std::string to_string(size_t indents = 1) {
        return indented_title_with_data("Range", range, indents) +
               (prefill->get_num_threads() == 0
                    ? to_string_stage("without prefill")
                    : to_string_stage("prefill parameters") + prefill->to_string(indents + 1)) +
               (warmUp->get_num_threads() == 0
                    ? to_string_stage("without warmUp")
                    : to_string_stage("warmUp parameters") + warmUp->to_string(indents + 1)) +
               to_string_stage("test parameters") + test->to_string(indents + 1);
    }

    ~BenchParameters() {
        delete test;
        delete prefill;
        delete warmUp;
        delete_data_map_builders();
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
    s.test = new Parameters(json["test"]);
    s.prefill = new Parameters(json["prefill"]);
    s.warmUp = new Parameters(json["warmUp"]);
}

}  // namespace microbench::workload
