//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_DEFAULT_ARGS_GENERATOR_H
#define SETBENCH_DEFAULT_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

// template<typename size_t>
class DefaultArgsGenerator : public ArgsGenerator {
private:
//    PAD;
    Distribution *distribution;
    DataMap<long long> *dataMap;
//    PAD;

    size_t next() {
        size_t index = distribution->next();
        return dataMap->get(index);
    }

public:
    DefaultArgsGenerator(DataMap<long long> *_dataMap, Distribution *_distribution)
            : dataMap(_dataMap), distribution(_distribution) {}


    size_t nextGet() {
        return next();
    }

    size_t nextInsert() {
        return next();
    }

    size_t nextRemove() {
        return next();
    }

    std::pair<size_t, size_t> nextRange() {
        size_t left = nextGet();
        size_t right = nextGet();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    std::vector<shared_ptr<DataMap<long long>>> getInternalDataMaps() {
        std::vector<std::shared_ptr<DataMap<long long>>> result;
        result.reserve(4);
        for (int i = 0; i<4; ++i) {
            result.emplace_back(dataMap);
        }
        return result;
    }

    ~DefaultArgsGenerator() {
        delete distribution;
        delete dataMap;
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
class DefaultArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;
public:
    DistributionBuilder *distributionBuilder = new UniformDistributionBuilder();
    DataMapBuilder *dataMapBuilder = new IdDataMapBuilder();

    DefaultArgsGeneratorBuilder *setDistributionBuilder(DistributionBuilder *_distributionBuilder) {
        distributionBuilder = _distributionBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        dataMapBuilder->init(_range);
        return this;
    }

    DefaultArgsGenerator *build(Random64 &_rng) override {
        return new DefaultArgsGenerator(dataMapBuilder->build(),
                                        distributionBuilder->build(_rng, range));
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "DefaultArgsGeneratorBuilder";
        j["distributionBuilder"] = *distributionBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        distributionBuilder = getDistributionFromJson(j["distributionBuilder"]);
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "DEFAULT", indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->toString(indents + 1);
        res += indented_title("Data Map", indents);
        res += dataMapBuilder->toString(indents + 1);
return res;

//        return indented_title_with_str_data("Type", "DEFAULT", indents)
//               + indented_title("Distribution", indents)
//               + distributionBuilder->toString(indents + 1)
//               + indented_title("Data Map", indents)
//               + dataMapBuilder->toString(indents + 1);
    }

    ~DefaultArgsGeneratorBuilder() override {
        delete distributionBuilder;
//        delete dataMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_DEFAULT_ARGS_GENERATOR_H
