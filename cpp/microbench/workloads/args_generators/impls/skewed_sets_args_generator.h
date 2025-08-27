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
    DataMap<long long> *dataMap;

    size_t nextWrite() {
        size_t index = writeSetBegins + writeDist->next();
        if (index >= range) {
            index -= range;
        }
        return dataMap->get(index);
    }

public:

    SkewedSetsArgsGenerator(size_t range, size_t writeSetBegins,
                            Distribution *readDist, Distribution *writeDist,
                            DataMap<long long> *dataMap)
            : range(range), writeSetBegins(writeSetBegins),
              readDist(readDist), writeDist(writeDist),
              dataMap(dataMap) {}

    size_t nextGet() override {
        return dataMap->get(readDist->next());
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

    std::vector<shared_ptr<DataMap<long long>>> getInternalDataMaps() {
        std::vector<std::shared_ptr<DataMap<long long>>> result;
        result.reserve(4);
        for (int i = 0; i<4; ++i) {
            result.emplace_back(dataMap);
        }
        return result;
    }

    ~SkewedSetsArgsGenerator() override {
        delete readDist;
        delete writeDist;
        delete dataMap;
    };


};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"

// template<typename size_t>
class SkewedSetsArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range;

    SkewedUniformDistributionBuilder *readDistBuilder = new SkewedUniformDistributionBuilder();
    SkewedUniformDistributionBuilder *writeDistBuilder = new SkewedUniformDistributionBuilder();

    DataMapBuilder *dataMapBuilder = new ArrayDataMapBuilder();

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

    SkewedSetsArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *setIntersection(double _intersection) {
        intersection = _intersection;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        dataMapBuilder->init(range);
        writeSetBegins = readDistBuilder->getHotLength(range) - range * intersection;
        return this;
    }

    SkewedSetsArgsGenerator *build(Random64 &_rng) override {
        return new SkewedSetsArgsGenerator(
                range, writeSetBegins,
                readDistBuilder->build(_rng, range),
                writeDistBuilder->build(_rng, range),
                dataMapBuilder->build()
        );
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "SkewedSetsArgsGeneratorBuilder";
        j["readDistBuilder"] = *readDistBuilder;
        j["writeDistBuilder"] = *writeDistBuilder;
        j["intersection"] = intersection;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        readDistBuilder = dynamic_cast<SkewedUniformDistributionBuilder *>(
                getDistributionFromJson(j["readDistBuilder"])
        );
        writeDistBuilder = dynamic_cast<SkewedUniformDistributionBuilder *>(
                getDistributionFromJson(j["writeDistBuilder"])
        );
        intersection = j["intersection"];
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);
    }

    std::string toString(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_SETS", indents)
               + indented_title_with_data("Intersection", intersection, indents)
               + indented_title("Read Distribution", indents)
               + readDistBuilder->toString(indents + 1)
               + indented_title("Write Distribution", indents)
               + writeDistBuilder->toString(indents + 1)
               + indented_title("Data Map", indents)
               + dataMapBuilder->toString(indents + 1);
    }

    ~SkewedSetsArgsGeneratorBuilder() override {
        delete readDistBuilder;
        delete writeDistBuilder;
//        delete dataMapBuilder; //TODO may delete twice
    };

};


#endif //SETBENCH_SKEWED_SETS_ARGS_GENERATOR_H
