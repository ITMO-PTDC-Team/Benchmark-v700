#ifndef SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H
#define SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/index_maps/index_map.h"

// template<typename size_t>
class RangeQueryArgsGenerator : public ArgsGenerator {
private:
//    PAD;
    Distribution *distribution;
    IndexMap *indexMap;
    size_t interval;
//    PAD;

public:
    RangeQueryArgsGenerator(IndexMap *_indexMap, Distribution *_distribution, size_t _interval)
            : indexMap(_indexMap), distribution(_distribution), interval(_interval) {}

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
        size_t left = indexMap->get(index);
        size_t right = indexMap->get(index + interval);
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    std::vector<shared_ptr<IndexMap>> getInternalIndexMaps() {
        std::vector<std::shared_ptr<IndexMap>> result;
        result.reserve(4);
        for (int i = 0; i<3; ++i) {
            result.emplace_back(nullptr);
        }
        result.emplace_back(indexMap);
        return result;
    }

    ~RangeQueryArgsGenerator() {
        delete distribution;
        delete indexMap;
    }
};


#include "workloads/distributions/distribution_builder.h"
#include "workloads/index_maps/index_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/index_maps/builders/id_index_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/index_maps/index_map_json_convector.h"
#include "globals_extern.h"

//template<typename size_t>
class RangeQueryArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;
    size_t interval;
public:
    DistributionBuilder *distributionBuilder = new UniformDistributionBuilder();
    IndexMapBuilder *indexMapBuilder = new IdIndexMapBuilder();

    RangeQueryArgsGeneratorBuilder *setDistributionBuilder(DistributionBuilder *_distributionBuilder) {
        distributionBuilder = _distributionBuilder;
        return this;
    }

    RangeQueryArgsGeneratorBuilder *setIndexMapBuilder(IndexMapBuilder *_indexMapBuilder) {
        indexMapBuilder = _indexMapBuilder;
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
        return new RangeQueryArgsGenerator(indexMapBuilder->build(),
                                           distributionBuilder->build(_rng, range), interval);
    }   

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "RangeQueryArgsGeneratorBuilder";
        j["interval"] = interval;
        j["distributionBuilder"] = *distributionBuilder;
        j["indexMapBuilder"] = *indexMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        interval = j["interval"];
        distributionBuilder = getDistributionFromJson(j["distributionBuilder"]);
        indexMapBuilder = getIndexMapFromJson(j["indexMapBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "RangeQuery", indents);
        res += indented_title_with_str_data("Interval", std::to_string(interval), indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->toString(indents + 1);
        res += indented_title("Index Map", indents);
        res += indexMapBuilder->toString(indents + 1);
        return res;
    }

    ~RangeQueryArgsGeneratorBuilder() override {
        delete distributionBuilder;
//        delete indexMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_RANGE_QUERY_ARGS_GENERATOR_H
