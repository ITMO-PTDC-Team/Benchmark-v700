//
// Created by Ravil Galiev on 06.04.2023.
//

#ifndef SETBENCH_DEFAULT_THREAD_LOOP_H
#define SETBENCH_DEFAULT_THREAD_LOOP_H

#include "workloads/thread_loops/thread_loop.h"
#include "workloads/args_generators/args_generator.h"
#include "workloads/thread_loops/ratio_thread_loop_parameters.h"

//template<typename K>
class DefaultThreadLoop : public ThreadLoop {
    PAD;
    double *cdf;
    Random64 &rng;
    PAD;
    ArgsGenerator<K> *argsGenerator;
    PAD;

public:
    DefaultThreadLoop(globals_t *_g, Random64 &_rng, size_t _threadId, StopCondition *_stopCondition, size_t _RQ_RANGE,
                      ArgsGenerator<K> *_argsGenerator,
                      RatioThreadLoopParameters &threadLoopParameters)
            : ThreadLoop(_g, _threadId, _stopCondition, _RQ_RANGE),
              rng(_rng), argsGenerator(_argsGenerator) {
        cdf = new double[3];
        cdf[0] = threadLoopParameters.INS_RATIO;
        cdf[1] = cdf[0] + threadLoopParameters.REM_RATIO;
        cdf[2] = cdf[1] + threadLoopParameters.RQ_RATIO;
    }

    void step() override {
        double op = (double) rng.next() / (double) rng.max_value;
        if (op < cdf[0]) { // insert
            K key = this->argsGenerator->nextInsert();
            this->executeInsert(key);
        } else if (op < cdf[1]) { // remove
            K key = this->argsGenerator->nextRemove();
            this->executeRemove(key);
        } else if (op < cdf[2]) { // range query
            std::pair<K, K> keys = this->argsGenerator->nextRange();
            this->executeRangeQuery(keys.first, keys.second);
        } else { // read
            K key = this->argsGenerator->nextGet();
            this->GET_FUNC(key);
        }
    }
};

#include "workloads/thread_loops/thread_loop_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/args_generator_json_convector.h"
#include "globals_extern.h"

//template<typename K>
struct DefaultThreadLoopBuilder : public ThreadLoopBuilder {
    RatioThreadLoopParameters parameters;

    ArgsGeneratorBuilder *argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();

    DefaultThreadLoopBuilder *setInsRatio(double insRatio) {
        parameters.INS_RATIO = insRatio;
        return this;
    }

    DefaultThreadLoopBuilder *setRemRatio(double delRatio) {
        parameters.REM_RATIO = delRatio;
        return this;
    }

    DefaultThreadLoopBuilder *setRqRatio(double rqRatio) {
        parameters.RQ_RATIO = rqRatio;
        return this;
    }

    DefaultThreadLoopBuilder *setArgsGeneratorBuilder(ArgsGeneratorBuilder *_argsGeneratorBuilder) {
        argsGeneratorBuilder = _argsGeneratorBuilder;
        return this;
    }

    DefaultThreadLoopBuilder *init(int range) override {
        ThreadLoopBuilder::init(range);
        argsGeneratorBuilder->init(range);
        return this;
    }

//    template<typename K>
    ThreadLoop *build(globals_t *_g, Random64 &_rng, size_t _threadId, StopCondition *_stopCondition) override {
        return new DefaultThreadLoop(_g, _rng, _threadId, _stopCondition, this->RQ_RANGE,
                                     argsGeneratorBuilder->build(_rng),
                                     parameters);
    }

    void toJson(nlohmann::json &json) const override {
        json["ClassName"] = "DefaultThreadLoopBuilder";
        json["parameters"] = parameters;
        json["argsGeneratorBuilder"] = *argsGeneratorBuilder;
    }


    void fromJson(const nlohmann::json &j) override {
        parameters = j["parameters"];
        argsGeneratorBuilder = getArgsGeneratorFromJson(j["argsGeneratorBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "Default", indents)
               + indented_title_with_data("INS_RATIO", parameters.INS_RATIO, indents)
               + indented_title_with_data("REM_RATIO", parameters.REM_RATIO, indents)
               + indented_title_with_data("RQ_RATIO", parameters.RQ_RATIO, indents)
               + indented_title("Args generator", indents)
               + argsGeneratorBuilder->toString(indents + 1);
    }

    ~DefaultThreadLoopBuilder() override {
        delete argsGeneratorBuilder;
    };
};


#endif //SETBENCH_DEFAULT_THREAD_LOOP_H
