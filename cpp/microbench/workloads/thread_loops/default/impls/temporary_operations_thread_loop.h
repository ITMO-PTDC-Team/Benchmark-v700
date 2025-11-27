//
// Created by Ravil Galiev on 16.08.2023.
//
#pragma once

#include "random_xoshiro256p.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/args_generators/args_generator_json_convector.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/thread_loops/default/normal_thread_loop.h"
#include "workloads/thread_loops/ratio_thread_loop_parameters.h"

namespace microbench::workload {

class TemporaryOperationThreadLoop : public NormalThreadLoop {
    PAD;
    double** cdf_;
    Random64& rng_;
    PAD;
    ArgsGenerator<K>* args_generator_;
    PAD;
    size_t time_;
    size_t pointer_;
    size_t stages_number_;
    size_t* stages_durations_;
    PAD;

    void update_pointer() {
        if (time_ >= stages_durations_[pointer_]) {
            time_ = 0;
            ++pointer_;
            if (pointer_ >= stages_number_) {
                pointer_ = 0;
            }
        }
        ++time_;
    }

public:
    TemporaryOperationThreadLoop(globals_t* g, Random64& rng, size_t thread_id,
                                 StopCondition* stop_condition, size_t rq_range,
                                 size_t stages_number, size_t* stages_durations,
                                 RatioThreadLoopParameters** ratios,
                                 ArgsGenerator<K>* args_generator)
        : NormalThreadLoop(g, thread_id, stop_condition, rq_range),
          rng_(rng),
          args_generator_(args_generator),
          stages_number_(stages_number),
          time_(0),
          pointer_(0) {
        cdf_ = new double*[stages_number];
        stages_durations_ = new size_t[stages_number];
        std::copy(stages_durations, stages_durations + stages_number, stages_durations_);

        for (size_t i = 0; i < stages_number; ++i) {
            cdf_[i] = new double[3];
            cdf_[i][0] = ratios[i]->INS_RATIO;
            cdf_[i][1] = cdf_[i][0] + ratios[i]->REM_RATIO;
            cdf_[i][2] = cdf_[i][1] + ratios[i]->RQ_RATIO;
        }
    }

    void step() override {
        update_pointer();

        double op = (double)rng_.next() / (double)rng_.max_value;
        if (op < cdf_[pointer_][0]) {  // insert
            K key = this->args_generator_->next_insert();
            this->execute_insert(key);
        } else if (op < cdf_[pointer_][1]) {  // remove
            K key = this->args_generator_->next_remove();
            this->execute_remove(key);
        } else if (op < cdf_[pointer_][2]) {  // range query
            std::pair<K, K> keys = this->args_generator_->next_range();
            this->execute_range_query(keys.first, keys.second);
        } else {  // read
            K key = this->args_generator_->next_get();
            this->GET_FUNC(key);
        }
    }
};

}  // namespace microbench::workload

#include "workloads/thread_loops/thread_loop_builder.h"

namespace microbench::workload {

struct TemporaryOperationsThreadLoopBuilder : public ThreadLoopBuilder {
    size_t stagesNumber = 0;
    size_t* stagesDurations;
    RatioThreadLoopParameters** ratios;

    ArgsGeneratorBuilder* argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();

    TemporaryOperationsThreadLoopBuilder* set_stages_number(const size_t stages_number) {
        stagesNumber = stages_number;
        ratios = new RatioThreadLoopParameters*[stages_number];
        stagesDurations = new size_t[stages_number];

        for (size_t i = 0; i < stages_number; ++i) {
            ratios[i] = new RatioThreadLoopParameters();
        }

        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_stage_duration(const size_t index,
                                                             size_t stage_duration) {
        assert(index < stagesNumber);
        stagesDurations[index] = stage_duration;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_stages_durations(size_t* stages_durations) {
        stagesDurations = stages_durations;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_ins_ratio(const size_t index, double ins_ratio) {
        assert(index < stagesNumber);
        ratios[index]->INS_RATIO = ins_ratio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_rem_ratio(const size_t index, double rem_ratio) {
        assert(index < stagesNumber);
        ratios[index]->REM_RATIO = rem_ratio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_rq_ratio(const size_t index, double rq_ratio) {
        assert(index < stagesNumber);
        ratios[index]->RQ_RATIO = rq_ratio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_ratios(const size_t index,
                                                     RatioThreadLoopParameters* ratio) {
        assert(index < stagesNumber);
        ratios[index] = ratio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_ratios(RatioThreadLoopParameters** ratios) {
        ratios = ratios;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* set_args_generator_builder(
        ArgsGeneratorBuilder* args_generator_builder) {
        argsGeneratorBuilder = args_generator_builder;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder* init(int range) override {
        ThreadLoopBuilder::init(range);
        argsGeneratorBuilder->init(range);
        return this;
    }

    TemporaryOperationThreadLoop* build(globals_t* g, Random64& rng, size_t tid,
                                        StopCondition* stop_condition) override {
        return new TemporaryOperationThreadLoop(g, rng, tid, stop_condition, this->RQ_RANGE,
                                                stagesNumber, stagesDurations, ratios,
                                                argsGeneratorBuilder->build(rng));
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "TemporaryOperationsThreadLoopBuilder";
        j["stagesNumber"] = stagesNumber;
        for (size_t i = 0; i < stagesNumber; ++i) {
            j["ratios"].push_back(*ratios[i]);
            j["stagesDurations"].push_back(stagesDurations[i]);
        }
        j["argsGeneratorBuilder"] = *argsGeneratorBuilder;
    }

    void from_json(const nlohmann::json& j) override {
        this->set_stages_number(j["stagesNumber"]);

        std::copy(std::begin(j["stagesDurations"]), std::end(j["stagesDurations"]),
                  stagesDurations);

        size_t i = 0;
        for (const auto& j_i : j["ratios"]) {
            ratios[i] = new RatioThreadLoopParameters(j_i);
            ++i;
        }

        argsGeneratorBuilder = get_args_generator_from_json(j["argsGeneratorBuilder"]);
    }

    std::string to_string(size_t indents) override {
        std::string result = indented_title_with_str_data("Type", "TEMPORARY_OPERATION", indents) +
                             indented_title_with_data("Stages number", stagesNumber, indents) +
                             indented_title("Stages Durations", indents);

        for (size_t i = 0; i < stagesNumber; ++i) {
            result += indented_title_with_data("Stage Duration " + std::to_string(i),
                                               stagesDurations[i], indents + 1);
        }

        result += indented_title("Ratios", indents);

        for (size_t i = 0; i < stagesNumber; ++i) {
            result += indented_title("Ratio " + std::to_string(i), indents + 1) +
                      ratios[i]->to_string(indents + 2);
        }

        result += indented_title("Args generator", indents) +
                  argsGeneratorBuilder->to_string(indents + 1);

        return result;
    }

    ~TemporaryOperationsThreadLoopBuilder() override {
        delete stagesDurations;
        delete[] ratios;
        delete argsGeneratorBuilder;
    };
};

}  // namespace microbench::workload
