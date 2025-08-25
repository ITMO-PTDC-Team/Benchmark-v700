//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_DEFAULT_ARGS_GENERATOR_H
#define SETBENCH_DEFAULT_ARGS_GENERATOR_H

#include <memory>
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

template<typename K>
class DefaultArgsGenerator : public ArgsGenerator<K> {
private:
//    PAD;
    std::shared_ptr<Distribution> distribution;
    std::shared_ptr<DataMap<K>> data;
//    PAD;

    K next() {
        size_t index = distribution->next();
        return data->get(index);
    }

public:
    DefaultArgsGenerator(std::shared_ptr<DataMap<K>> _data, std::shared_ptr<Distribution> _distribution)
            : data(_data), distribution(_distribution) {}


    K nextGet() {
        return next();
    }

    K nextInsert() {
        return next();
    }

    K nextRemove() {
        return next();
    }

    std::pair<K, K> nextRange() {
        K left = nextGet();
        K right = nextGet();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~DefaultArgsGenerator() = default;
};


#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

//template<typename K>
class DefaultArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;
public:
    std::shared_ptr<DistributionBuilder> distributionBuilder = std::make_shared<UniformDistributionBuilder>();
    std::shared_ptr<DataMapBuilder> dataMapBuilder = std::make_shared<IdDataMapBuilder>();

    DefaultArgsGeneratorBuilder *setDistributionBuilder(std::shared_ptr<DistributionBuilder> _distributionBuilder) {
        distributionBuilder = _distributionBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *setDataMapBuilder(std::shared_ptr<DataMapBuilder> _dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        dataMapBuilder->init(_range);
        return this;
    }

    std::shared_ptr<ArgsGenerator<K>> build(Random64 &_rng) override {
        return std::shared_ptr<DefaultArgsGenerator<K>>(new DefaultArgsGenerator<K>(dataMapBuilder->build(),
                                           distributionBuilder->build(_rng, range)));
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

    ~DefaultArgsGeneratorBuilder() override = default;
};


#endif //SETBENCH_DEFAULT_ARGS_GENERATOR_H
