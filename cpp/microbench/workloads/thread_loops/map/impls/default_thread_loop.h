//
// Created by Ravil Galiev on 06.04.2023.
//
#pragma once

#include "plaf.h"
#include "random_xoshiro256p.h"
#include "workloads/thread_loops/map/map_thread_loop.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/thread_loops/ratio_thread_loop_parameters.h"

namespace microbench::workload::map {

class DefaultThreadLoop : public MapThreadLoop {
    PAD;
    double* cdf_;
    Random64& rng_;
    PAD;
    ArgsGenerator<K>* args_generator_;
    PAD;

public:
    DefaultThreadLoop(globals_t* g, Random64& rng, size_t thread_id, StopCondition* stop_condition,
                      size_t rq_range, ArgsGenerator<K>* args_generator,
                      RatioThreadLoopParameters& thread_loop_parameters)
        : MapThreadLoop(g, thread_id, stop_condition, rq_range),
          rng_(rng),
          args_generator_(args_generator) {
        cdf_ = new double[3];
        cdf_[0] = thread_loop_parameters.INS_RATIO;
        cdf_[1] = cdf_[0] + thread_loop_parameters.REM_RATIO;
        cdf_[2] = cdf_[1] + thread_loop_parameters.RQ_RATIO;
    }

    void step() override {
        double op = (double)rng_.next() / (double)rng_.max_value;
        if (op < cdf_[0]) {  // insert
            K key = this->args_generator_->next_insert();
            this->execute_insert(key);
        } else if (op < cdf_[1]) {  // remove
            K key = this->args_generator_->next_remove();
            this->execute_remove(key);
        } else if (op < cdf_[2]) {  // range query
            std::pair<K, K> keys = this->args_generator_->next_range();
            this->execute_range_query(keys.first, keys.second);
        } else {  // read
            K key = this->args_generator_->next_get();
            this->GET_FUNC(key);
        }
    }
};

}  // namespace microbench::workload::map

#include "workloads/thread_loops/thread_loop_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/args_generator_json_convector.h"
#include "globals_extern.h"

namespace microbench::workload::map {

struct DefaultThreadLoopBuilder : public ThreadLoopBuilder {
    RatioThreadLoopParameters parameters;

    ArgsGeneratorBuilder* argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();

    DefaultThreadLoopBuilder* set_ins_ratio(double ins_ratio) {
        parameters.INS_RATIO = ins_ratio;
        return this;
    }

    DefaultThreadLoopBuilder* set_rem_ratio(double del_ratio) {
        parameters.REM_RATIO = del_ratio;
        return this;
    }

    DefaultThreadLoopBuilder* set_rq_ratio(double rq_ratio) {
        parameters.RQ_RATIO = rq_ratio;
        return this;
    }

    DefaultThreadLoopBuilder* set_args_generator_builder(
        ArgsGeneratorBuilder* args_generator_builder) {
        argsGeneratorBuilder = args_generator_builder;
        return this;
    }

    DefaultThreadLoopBuilder* init(int range) override {
        ThreadLoopBuilder::init(range);
        argsGeneratorBuilder->init(range);
        return this;
    }

    //    template<typename K>
    ThreadLoop* build(globals_t* g, Random64& rng, size_t thread_id,
                      StopCondition* stop_condition) override {
        return new DefaultThreadLoop(g, rng, thread_id, stop_condition, this->RQ_RANGE,
                                     argsGeneratorBuilder->build(rng), parameters);
    }

    void to_json(nlohmann::json& json) const override {
        json["ClassName"] = "DefaultThreadLoopBuilder";
        json["parameters"] = parameters;
        json["argsGeneratorBuilder"] = *argsGeneratorBuilder;
    }

    void from_json(const nlohmann::json& j) override {
        parameters = j["parameters"];
        argsGeneratorBuilder = get_args_generator_from_json(j["argsGeneratorBuilder"]);
    }

    std::string to_string(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Default", indents) +
               indented_title_with_data("INS_RATIO", parameters.INS_RATIO, indents) +
               indented_title_with_data("REM_RATIO", parameters.REM_RATIO, indents) +
               indented_title_with_data("RQ_RATIO", parameters.RQ_RATIO, indents) +
               indented_title("Args generator", indents) +
               argsGeneratorBuilder->to_string(indents + 1);
    }

    ~DefaultThreadLoopBuilder() override {
        delete argsGeneratorBuilder;
    };
};

}  // namespace microbench::workload::map
