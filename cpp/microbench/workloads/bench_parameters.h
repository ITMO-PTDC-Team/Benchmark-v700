//
// Created by Ravil Galiev on 21.07.2023.
//
#pragma once

#include <cassert>
#include "parameters.h"

namespace microbench::workload {

struct BenchParameters {
    size_t range;

    std::optional<Parameters> test{};
    std::optional<Parameters> prefill{};
    std::optional<Parameters> warmUp{};

    BenchParameters() {
        range = 2048;
    }

    BenchParameters& create_default_prefill(size_t thread_num) {
        assert(!prefill.has_value());
        prefill.emplace();
        prefill->set_stop_condition(std::make_shared<OperationCounter>(range / 2))
            .add_thread_loop_builder(std::make_shared<PrefillInsertThreadLoopBuilder>(),
                                     thread_num);
        return *this;
    }

    BenchParameters& create_default_prefill() {
        assert(!prefill.has_value());
        prefill.emplace();
        prefill->stopCondition = std::make_shared<OperationCounter>(range / 2);
        prefill->add_thread_loop_builder(std::make_shared<PrefillInsertThreadLoopBuilder>(), 1);
        return create_default_prefill(1);
    }

    BenchParameters& set_range(size_t range) {
        this->range = range;
        return *this;
    }

    BenchParameters& set_test(Parameters test) {
        this->test = std::move(test);
        return *this;
    }

    BenchParameters& set_prefill(Parameters prefill) {
        this->prefill = std::move(prefill);
        return *this;
    }

    BenchParameters& set_warm_up(Parameters warm_up) {
        this->warmUp = std::move(warm_up);
        return *this;
    }

    size_t get_total_threads() {
        return (prefill.has_value() ? prefill->get_num_threads() : 0) +
               (warmUp.has_value() ? warmUp->get_num_threads() : 0) +
               (test.has_value() ? test->get_num_threads() : 0);
    }

    size_t get_max_threads() {
        return std::max({(prefill.has_value() ? prefill->get_num_threads() : 0),
                         (warmUp.has_value() ? warmUp->get_num_threads() : 0),
                         (test.has_value() ? test->get_num_threads() : 0)});
    }

    void init() {
        init_data_map_builders(range);
        if (prefill.has_value()) {
            prefill->init(range);
        }
        if (warmUp.has_value()) {
            warmUp->init(range);
        }
        if (test.has_value()) {
            test->init(range);
        }
    }

    std::string to_string(size_t indents = 1) {
        return indented_title_with_data("Range", range, indents) +
               (!prefill.has_value()
                    ? to_string_stage("without prefill")
                    : to_string_stage("prefill parameters") + prefill->to_string(indents + 1)) +
               (!warmUp.has_value()
                    ? to_string_stage("without warmUp")
                    : to_string_stage("warmUp parameters") + warmUp->to_string(indents + 1)) +
               to_string_stage("test parameters") + test->to_string(indents + 1);
    }

    ~BenchParameters() = default;
};

void to_json(nlohmann::json& json, const BenchParameters& s) {
    json["range"] = s.range;
    if (s.test.has_value()) {
        json["test"] = s.test;
    }
    if (s.prefill.has_value()) {
        json["prefill"] = s.prefill;
    }
    if (s.warmUp.has_value()) {
        json["warmUp"] = s.warmUp;
    }
}

void from_json(const nlohmann::json& json, BenchParameters& s) {
    s.range = json["range"];
    if (json.contains("test")) {
        s.test = Parameters(json["test"]);
    }
    if (json.contains("prefill")) {
        s.prefill = Parameters(json["prefill"]);
    }
    if (json.contains("warmUp")) {
        s.warmUp = Parameters(json["warmUp"]);
    }
}

}  // namespace microbench::workload
