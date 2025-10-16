//
// Created by Ravil Galiev on 08.08.2023.
//

#ifndef SETBENCH_SKEWED_INSERT_ARGS_GENERATOR_H
#define SETBENCH_SKEWED_INSERT_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"
#include "errors.h"

// template<typename size_t>
class SkewedInsertArgsGenerator : public ArgsGenerator {
//    PAD;
    size_t skewedLength;
    size_t insertedNumber;
    Distribution *distribution;
    PAD;
    IndexMap *indexMap;
    PAD;

public:

    SkewedInsertArgsGenerator(size_t skewedLength, Distribution *distribution, IndexMap *indexMap)
            : insertedNumber(0), skewedLength(skewedLength),
              distribution(distribution), indexMap(indexMap) {}

    size_t nextGet() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    size_t nextInsert() override {
        size_t value;
        if (insertedNumber < skewedLength) {
            value = indexMap->get(insertedNumber++);
        } else {
            value = indexMap->get(skewedLength + distribution->next());
        }
        return value;
    }

    size_t nextRemove() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    std::pair <size_t, size_t> nextRange() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    std::vector<shared_ptr<IndexMap>> getInternalIndexMaps() {
        std::vector<std::shared_ptr<IndexMap>> result;
        result.reserve(4);
        for (int i = 0; i<4; ++i) {
            if (i == 1) {
                result.emplace_back(indexMap);
                continue;
            }
            result.emplace_back(nullptr);
        }
        return result;
    }

    ~SkewedInsertArgsGenerator() override {
        delete distribution;
        delete indexMap;
    };


};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/index_maps/index_map_builder.h"
#include "workloads/index_maps/builders/array_index_map_builder.h"

// template<typename size_t>
class SkewedInsertArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range;

    DistributionBuilder *distBuilder = new UniformDistributionBuilder();

    IndexMapBuilder *indexMapBuilder = new ArrayIndexMapBuilder();

    double skewedSize = 0;

    size_t skewedLength;

public:
    SkewedInsertArgsGeneratorBuilder *setSkewedSize(double _skewedSize) {
        skewedSize = _skewedSize;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder *setDistributionBuilder(DistributionBuilder *_distBuilder) {
        distBuilder = _distBuilder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder *setIndexMapBuilder(IndexMapBuilder *_indexMapBuilder) {
        indexMapBuilder = _indexMapBuilder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        indexMapBuilder->init(range);
        skewedLength = (size_t) (_range * skewedSize);
        return this;
    }

    SkewedInsertArgsGenerator *build(Random64 &_rng) override {
        return new SkewedInsertArgsGenerator(
                skewedLength,
                distBuilder->build(_rng, range - skewedLength),
                indexMapBuilder->build()
        );
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "SkewedInsertArgsGeneratorBuilder";
        j["distributionBuilder"] = *distBuilder;
        j["skewedSize"] = skewedSize;
        j["indexMapBuilder"] = *indexMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        distBuilder = getDistributionFromJson(j["distributionBuilder"]);
        skewedSize = j["skewedSize"];
        indexMapBuilder = getIndexMapFromJson(j["indexMapBuilder"]);
    }

    std::string toString(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_INSERT", indents)
               + indented_title_with_data("Skewed size", skewedSize, indents)
               + indented_title("Distribution", indents)
               + distBuilder->toString(indents + 1)
               + indented_title("Index Map", indents)
               + indexMapBuilder->toString(indents + 1);
    }

    ~SkewedInsertArgsGeneratorBuilder() override {
        delete distBuilder;
//        delete indexMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_SKEWED_INSERT_ARGS_GENERATOR_H
