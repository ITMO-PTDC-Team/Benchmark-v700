//
// Created by Ravil Galiev on 16.08.2023.
//

#ifndef SETBENCH_TEMPORARY_OPERATIONS_THREAD_LOOP_H
#define SETBENCH_TEMPORARY_OPERATIONS_THREAD_LOOP_H

#include "workloads/thread_loops/thread_loop.h"
#include "workloads/thread_loops/ratio_thread_loop_parameters.h"

//template<typename K>
class TemporaryOperationThreadLoop : public ThreadLoop {
    PAD;
    double **cdf;
    Random64 &rng;
    PAD;
    ArgsGenerator<K> *argsGenerator;
    PAD;
    size_t time;
    size_t pointer;
    size_t stagesNumber;
    size_t *stagesDurations;
    PAD;

    void update_pointer() {
        if (time >= stagesDurations[pointer]) {
            time = 0;
            ++pointer;
            if (pointer >= stagesNumber) {
                pointer = 0;
            }
        }
        ++time;
    }

public:
    TemporaryOperationThreadLoop(globals_t *g, Random64 &_rng, size_t threadId, StopCondition *stopCondition,
                                 size_t rqRange,
                                 size_t _stagesNumber,
                                 size_t *_stagesDurations,
                                 RatioThreadLoopParameters **ratios,
                                 ArgsGenerator<K> *_argsGenerator)
            : ThreadLoop(g, threadId, stopCondition, rqRange),
              rng(_rng),
              argsGenerator(_argsGenerator),
              stagesNumber(_stagesNumber),
              time(0),
              pointer(0) {
        cdf = new double *[_stagesNumber];
        stagesDurations = new size_t[_stagesNumber];
        std::copy(_stagesDurations, _stagesDurations + _stagesNumber, stagesDurations);

        for (size_t i = 0; i < _stagesNumber; ++i) {
            cdf[i] = new double[3];
            cdf[i][0] = ratios[i]->INS_RATIO;
            cdf[i][1] = cdf[i][0] + ratios[i]->REM_RATIO;
            cdf[i][2] = cdf[i][1] + ratios[i]->RQ_RATIO;
        }
    }

    void step() override {
        update_pointer();

        double op = (double) rng.next() / (double) rng.max_value;
        if (op < cdf[pointer][0]) { // insert
            K key = this->argsGenerator->nextInsert();
            this->executeInsert(key);
        } else if (op < cdf[pointer][1]) { // remove
            K key = this->argsGenerator->nextRemove();
            this->executeRemove(key);
        } else if (op < cdf[pointer][2]) { // range query
            std::pair<K, K> keys = this->argsGenerator->nextRange();
            this->executeRangeQuery(keys.first, keys.second);
        } else { // read
            K key = this->argsGenerator->nextGet();
            this->GET_FUNC(key);
        }
    }
};

#include "workloads/thread_loops/thread_loop_builder.h"
#include "default_thread_loop.h"


struct TemporaryOperationsThreadLoopBuilder : public ThreadLoopBuilder {
    size_t stagesNumber = 0;
    size_t *stagesDurations;
    RatioThreadLoopParameters **ratios;

    ArgsGeneratorBuilder *argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();

    TemporaryOperationsThreadLoopBuilder *setStagesNumber(const size_t _stagesNumber) {
        stagesNumber = _stagesNumber;
        ratios = new RatioThreadLoopParameters *[_stagesNumber];
        stagesDurations = new size_t[_stagesNumber];

        for (size_t i = 0; i < _stagesNumber; ++i) {
            ratios[i] = new RatioThreadLoopParameters();
        }

        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setStageDuration(const size_t index, size_t stageDuration) {
        assert(index < stagesNumber);
        stagesDurations[index] = stageDuration;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setStagesDurations(size_t *_stagesDurations) {
        stagesDurations = _stagesDurations;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setInsRatio(const size_t index, double insRatio) {
        assert(index < stagesNumber);
        ratios[index]->INS_RATIO = insRatio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setRemRatio(const size_t index, double remRatio) {
        assert(index < stagesNumber);
        ratios[index]->REM_RATIO = remRatio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setRqRatio(const size_t index, double rqRatio) {
        assert(index < stagesNumber);
        ratios[index]->RQ_RATIO = rqRatio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setRatios(const size_t index, RatioThreadLoopParameters *ratio) {
        assert(index < stagesNumber);
        ratios[index] = ratio;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setRatios(RatioThreadLoopParameters **_ratios) {
        ratios = _ratios;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *setArgsGeneratorBuilder(ArgsGeneratorBuilder *_argsGeneratorBuilder) {
        argsGeneratorBuilder = _argsGeneratorBuilder;
        return this;
    }

    TemporaryOperationsThreadLoopBuilder *init(int range) override {
        ThreadLoopBuilder::init(range);
        argsGeneratorBuilder->init(range);
        return this;
    }

    TemporaryOperationThreadLoop *
    build(globals_t *_g, Random64 &_rng, size_t _tid, StopCondition *_stopCondition) override {
        return new TemporaryOperationThreadLoop(_g, _rng, _tid, _stopCondition, this->RQ_RANGE,
                                                stagesNumber, stagesDurations, ratios,
                                                argsGeneratorBuilder->build(_rng));
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "TemporaryOperationsThreadLoopBuilder";
        j["stagesNumber"] = stagesNumber;
        for (size_t i = 0; i < stagesNumber; ++i) {
            j["ratios"].push_back(*ratios[i]);
            j["stagesDurations"].push_back(stagesDurations[i]);
        }
        j["argsGeneratorBuilder"] = *argsGeneratorBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        this->setStagesNumber(j["stagesNumber"]);

        std::copy(std::begin(j["stagesDurations"]), std::end(j["stagesDurations"]), stagesDurations);

        size_t i = 0;
        for (const auto &j_i: j["ratios"]) {
            ratios[i] = new RatioThreadLoopParameters(j_i);
            ++i;
        }

        argsGeneratorBuilder = getArgsGeneratorFromJson(j["argsGeneratorBuilder"]);
    }

    std::string toString(size_t indents) override {
        std::string result =
                indented_title_with_str_data("Type", "TEMPORARY_OPERATION", indents)
                + indented_title_with_data("Stages number", stagesNumber, indents)
                + indented_title("Stages Durations", indents);

        for (size_t i = 0; i < stagesNumber; ++i) {
            result += indented_title_with_data("Stage Duration " + std::to_string(i),
                                               stagesDurations[i], indents + 1);
        }

        result += indented_title("Ratios", indents);

        for (size_t i = 0; i < stagesNumber; ++i) {
            result += indented_title("Ratio " + std::to_string(i), indents + 1)
                      + ratios[i]->toString(indents + 2);
        }

        result +=
                indented_title("Args generator", indents)
                + argsGeneratorBuilder->toString(indents + 1);

        return result;
    }

    ~TemporaryOperationsThreadLoopBuilder() override {
        delete stagesDurations;
        delete[] ratios;
        delete argsGeneratorBuilder;
    };
};


#endif //SETBENCH_TEMPORARY_OPERATIONS_THREAD_LOOP_H
