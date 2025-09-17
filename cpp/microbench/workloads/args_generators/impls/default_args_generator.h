//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_DEFAULT_ARGS_GENERATOR_H
#define SETBENCH_DEFAULT_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/index_maps/index_map.h"

// template<typename size_t>
class DefaultArgsGenerator : public ArgsGenerator {
private:
//    PAD;
    Distribution *distribution;
    IndexMap *indexMap;
//    PAD;

    size_t next() {
        size_t index = distribution->next();
        return indexMap->get(index);
    }

public:
    DefaultArgsGenerator(IndexMap *_indexMap, Distribution *_distribution)
            : indexMap(_indexMap), distribution(_distribution) {}


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

    std::vector<shared_ptr<IndexMap>> getInternalIndexMaps() {
        std::vector<std::shared_ptr<IndexMap>> result;
        result.reserve(4);
        for (int i = 0; i<4; ++i) {
            result.emplace_back(indexMap);
        }
        return result;
    }

    ~DefaultArgsGenerator() {
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
class DefaultArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;
public:
    DistributionBuilder *distributionBuilder = new UniformDistributionBuilder();
    IndexMapBuilder *indexMapBuilder = new IdIndexMapBuilder();

    DefaultArgsGeneratorBuilder *setDistributionBuilder(DistributionBuilder *_distributionBuilder) {
        distributionBuilder = _distributionBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *setIndexMapBuilder(IndexMapBuilder *_indexMapBuilder) {
        indexMapBuilder = _indexMapBuilder;
        return this;
    }

    DefaultArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        indexMapBuilder->init(_range);
        return this;
    }

    DefaultArgsGenerator *build(Random64 &_rng) override {
        return new DefaultArgsGenerator(indexMapBuilder->build(),
                                        distributionBuilder->build(_rng, range));
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "DefaultArgsGeneratorBuilder";
        j["distributionBuilder"] = *distributionBuilder;
        j["indexMapBuilder"] = *indexMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        distributionBuilder = getDistributionFromJson(j["distributionBuilder"]);
        indexMapBuilder = getIndexMapFromJson(j["indexMapBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "DEFAULT", indents);
        res += indented_title("Distribution", indents);
        res += distributionBuilder->toString(indents + 1);
        res += indented_title("Index Map", indents);
        res += indexMapBuilder->toString(indents + 1);
return res;

//        return indented_title_with_str_data("Type", "DEFAULT", indents)
//               + indented_title("Distribution", indents)
//               + distributionBuilder->toString(indents + 1)
//               + indented_title("Index Map", indents)
//               + indexMapBuilder->toString(indents + 1);
    }

    ~DefaultArgsGeneratorBuilder() override {
        delete distributionBuilder;
//        delete indexMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_DEFAULT_ARGS_GENERATOR_H
