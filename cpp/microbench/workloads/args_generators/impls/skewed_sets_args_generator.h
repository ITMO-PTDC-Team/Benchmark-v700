//
// Created by Ravil Galiev on 08.08.2023.
//

#ifndef SETBENCH_SKEWED_SETS_ARGS_GENERATOR_H
#define SETBENCH_SKEWED_SETS_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"

// template<typename size_t>
class SkewedSetsArgsGenerator : public ArgsGenerator {
//    PAD;
    size_t range;
    size_t writeSetBegins;
    Distribution *readDist;
    Distribution *writeDist;
    IndexMap *indexMap;

    size_t nextWrite() {
        size_t index = writeSetBegins + writeDist->next();
        if (index >= range) {
            index -= range;
        }
        return indexMap->get(index);
    }

public:

    SkewedSetsArgsGenerator(size_t range, size_t writeSetBegins,
                            Distribution *readDist, Distribution *writeDist,
                            IndexMap *indexMap)
            : range(range), writeSetBegins(writeSetBegins),
              readDist(readDist), writeDist(writeDist),
              indexMap(indexMap) {}

    size_t nextGet() override {
        return indexMap->get(readDist->next());
    }

    size_t nextInsert() override {
        return nextWrite();
    }

    size_t nextRemove() override {
        return nextWrite();
    }

    std::pair<size_t, size_t> nextRange() override {
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

    ~SkewedSetsArgsGenerator() override {
        delete readDist;
        delete writeDist;
        delete indexMap;
    };


};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "workloads/index_maps/index_map_builder.h"
#include "workloads/index_maps/builders/array_index_map_builder.h"

// template<typename size_t>
class SkewedSetsArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range;

    SkewedUniformDistributionBuilder *readDistBuilder = new SkewedUniformDistributionBuilder();
    SkewedUniformDistributionBuilder *writeDistBuilder = new SkewedUniformDistributionBuilder();

    IndexMapBuilder *indexMapBuilder = new ArrayIndexMapBuilder();

    double intersection = 0;
    size_t writeSetBegins;

public:
    SkewedSetsArgsGeneratorBuilder *setReadHotSize(double _hotSize) {
        readDistBuilder->setHotSize(_hotSize);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setReadHotProb(double _hotProb) {
        readDistBuilder->setHotRatio(_hotProb);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setWriteHotSize(double _hotSize) {
        writeDistBuilder->setHotSize(_hotSize);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setWriteHotProb(double _hotProb) {
        writeDistBuilder->setHotRatio(_hotProb);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setIndexMapBuilder(IndexMapBuilder *_indexMapBuilder) {
        indexMapBuilder = _indexMapBuilder;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setIntersection(double _intersection) {
        intersection = _intersection;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        indexMapBuilder->init(range);
        writeSetBegins = readDistBuilder->getHotLength(range) - range * intersection;
        return this;
    }

    SkewedSetsArgsGenerator *build(Random64 &_rng) override {
        return new SkewedSetsArgsGenerator(
                range, writeSetBegins,
                readDistBuilder->build(_rng, range),
                writeDistBuilder->build(_rng, range),
                indexMapBuilder->build()
        );
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "SkewedSetsArgsGeneratorBuilder";
        j["readDistBuilder"] = *readDistBuilder;
        j["writeDistBuilder"] = *writeDistBuilder;
        j["intersection"] = intersection;
        j["indexMapBuilder"] = *indexMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        readDistBuilder = dynamic_cast<SkewedUniformDistributionBuilder *>(
                getDistributionFromJson(j["readDistBuilder"])
        );
        writeDistBuilder = dynamic_cast<SkewedUniformDistributionBuilder *>(
                getDistributionFromJson(j["writeDistBuilder"])
        );
        intersection = j["intersection"];
        indexMapBuilder = getIndexMapFromJson(j["indexMapBuilder"]);
    }

    std::string toString(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_SETS", indents)
               + indented_title_with_data("Intersection", intersection, indents)
               + indented_title("Read Distribution", indents)
               + readDistBuilder->toString(indents + 1)
               + indented_title("Write Distribution", indents)
               + writeDistBuilder->toString(indents + 1)
               + indented_title("Index Map", indents)
               + indexMapBuilder->toString(indents + 1);
    }

    ~SkewedSetsArgsGeneratorBuilder() override {
        delete readDistBuilder;
        delete writeDistBuilder;
//        delete indexMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_SKEWED_SETS_ARGS_GENERATOR_H
