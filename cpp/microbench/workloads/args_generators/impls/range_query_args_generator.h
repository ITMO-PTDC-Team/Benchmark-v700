#ifndef SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H
#define SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

// template<typename size_t>
class RangeQueryArgsGenerator : public ArgsGenerator {
private:
//    PAD;
    Distribution *distribution;
    DataMap<long long> *data;
    size_t interval;
//    PAD;

public:
    RangeQueryArgsGenerator(DataMap<long long> *_data, Distribution *_distribution, size_t _interval)
            : data(_data), distribution(_distribution), interval(_interval) {}

    size_t nextGet() {
        setbench_error("Operation not supported");
    }

    size_t nextInsert() {
        setbench_error("Operation not supported");
    }

    size_t nextRemove() {
        setbench_error("Operation not supported");
    }

    std::pair<size_t, size_t> nextRange() {
        size_t index = distribution->next();
        size_t left = data->get(index);
        size_t right = data->get(index + interval);
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~RangeQueryArgsGenerator() {
        delete distribution;
        delete data;
    }
};


#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

//template<typename size_t>
class RangeQueryArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;
    size_t interval;
public:
    DistributionBuilder *distributionBuilder = new UniformDistributionBuilder();
    DataMapBuilder *dataMapBuilder = new IdDataMapBuilder();

    RangeQueryArgsGeneratorBuilder *setDistributionBuilder(DistributionBuilder *_distributionBuilder) {
        distributionBuilder = _distributionBuilder;
        return this;
    }

    RangeQueryArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }
    
    RangeQueryArgsGeneratorBuilder *setInterval(size_t _interval) {
        interval = _interval;
        return this;
    }

    RangeQueryArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
        return this;
    }

    RangeQueryArgsGenerator *build(Random64 &_rng) override {
        return new RangeQueryArgsGenerator(dataMapBuilder->build(),
                                           distributionBuilder->build(_rng, range), interval);
    }   

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "RangeQueryArgsGeneratorBuilder";
        j["interval"] = interval;
        j["distributionBuilder"] = *distributionBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        interval = j["interval"];
        distributionBuilder = getDistributionFromJson(j["distributionBuilder"]);
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "RangeQuery", indents);
        res += indented_title_with_str_data("Interval", std::to_string(interval), indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->toString(indents + 1);
        res += indented_title("Data Map", indents);
        res += dataMapBuilder->toString(indents + 1);
        return res;
    }

    ~RangeQueryArgsGeneratorBuilder() override {
        delete distributionBuilder;
//        delete dataMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H
