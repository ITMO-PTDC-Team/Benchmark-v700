//
// Created by Ravil Galiev on 20.09.2023.
//

#ifndef SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H
#define SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

// template<typename size_t>
class LeafsHandshakeArgsGenerator : public ArgsGenerator {
    size_t range;

    Distribution *readDistribution;
    MutableDistribution *insertDistribution;
    Distribution *removeDistribution;
    Random64 &rng;
    PAD;
    std::atomic<size_t> *deletedValue;
    PAD;

    DataMap<long long> *readDataMap;
    DataMap<long long> *removeDataMap;

public:
    LeafsHandshakeArgsGenerator(Random64 &rng, size_t range, std::atomic<size_t> *deletedValue,
                                Distribution *readDistribution, MutableDistribution *insertDistribution,
                                Distribution *removeDistribution, DataMap<long long> *readDataMap, DataMap<long long> *removeDataMap) :
            range(range),
            readDistribution(readDistribution),
            insertDistribution(insertDistribution),
            removeDistribution(removeDistribution),
            rng(rng), deletedValue(deletedValue),
            readDataMap(readDataMap),
            removeDataMap(removeDataMap) {}

    size_t nextGet() {
        return readDataMap->get(readDistribution->next());
    }

    size_t nextInsert() {
        size_t localDeletedValue = *deletedValue;

        size_t value;

        bool isRight = rng.nextDouble() >= 0.5;

        if (localDeletedValue == 1 || (isRight && localDeletedValue != range)) {
            value = localDeletedValue + insertDistribution->next(range - localDeletedValue) + 1;
        } else {
            value = localDeletedValue - insertDistribution->next(localDeletedValue - 1) - 1;
        }

        return value;
    }

    size_t nextRemove() {
        size_t localDeletedValue = *deletedValue;
        size_t value = removeDataMap->get(removeDistribution->next());

        //todo learn the difference between all kinds of weakCompareAndSet
        deletedValue->compare_exchange_weak(localDeletedValue, value);

        return value;
    }

    std::pair<size_t, size_t> nextRange() {
        setbench_error("Unsupported operation -- nextRange")
    }

    std::vector<shared_ptr<DataMap<long long>>> getInternalDataMaps() {
        std::vector<std::shared_ptr<DataMap<long long>>> result;
        result.reserve(4);
        result.emplace_back(readDataMap);
        result.emplace_back(readDataMap);
        result.emplace_back(removeDataMap);
        result.emplace_back(nullptr);
        return result;
    }

    ~LeafsHandshakeArgsGenerator() {
        delete readDataMap;
        delete removeDataMap;
        delete readDistribution;
        delete insertDistribution;
        delete removeDistribution;
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
class LeafsHandshakeArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;

    DistributionBuilder *readDistBuilder = new UniformDistributionBuilder();
    MutableDistributionBuilder *insertDistBuilder = new ZipfianDistributionBuilder();
    DistributionBuilder *removeDistBuilder = new UniformDistributionBuilder();

    DataMapBuilder *readDataMapBuilder = new IdDataMapBuilder();
    DataMapBuilder *removeDataMapBuilder = new IdDataMapBuilder();
    std::atomic<size_t> *deletedValue;

public:

    LeafsHandshakeArgsGeneratorBuilder *setReadDistBuilder(DistributionBuilder *_readDistBuilder) {
        readDistBuilder = _readDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setInsertDistBuilder(MutableDistributionBuilder *_insertDistBuilder) {
        insertDistBuilder = _insertDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setRemoveDistBuilder(DistributionBuilder *_removeDistBuilder) {
        removeDistBuilder = _removeDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setReadDataMapBuilder(DataMapBuilder *_readDataMapBuilder) {
        readDataMapBuilder = _readDataMapBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setRemoveDataMapBuilder(DataMapBuilder *_removeDataMapBuilder) {
        removeDataMapBuilder = _removeDataMapBuilder;
        return this;
    }


    LeafsHandshakeArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;
//        readDataMapBuilder->init(_range);
//        removeDataMapBuilder->init(_range);
        deletedValue = new std::atomic<size_t>(range / 2);

        return this;
    }

    LeafsHandshakeArgsGenerator *build(Random64 &_rng) override {
        return new LeafsHandshakeArgsGenerator(_rng, range, deletedValue,
                                                  readDistBuilder->build(_rng, range),
                                                  insertDistBuilder->build(_rng),
                                                  removeDistBuilder->build(_rng, range),
                                                  readDataMapBuilder->build(),
                                                  removeDataMapBuilder->build());
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "LeafsHandshakeArgsGeneratorBuilder";
        j["readDistBuilder"] = *readDistBuilder;
        j["insertDistBuilder"] = *insertDistBuilder;
        j["removeDistBuilder"] = *removeDistBuilder;
        j["readDataMapBuilder"] = *readDataMapBuilder;
        j["removeDataMapBuilder"] = *removeDataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        readDistBuilder = getDistributionFromJson(j["readDistBuilder"]);
        insertDistBuilder = getMutableDistributionFromJson(j["insertDistBuilder"]);
        removeDistBuilder = getDistributionFromJson(j["removeDistBuilder"]);
        readDataMapBuilder = getDataMapFromJson(j["readDataMapBuilder"]);
        removeDataMapBuilder = getDataMapFromJson(j["removeDataMapBuilder"]);
    }

    std::string toString(size_t indents = 1) override {
        return indented_title_with_str_data("Type", "LEAFS_HANDSHAKE", indents)
               + indented_title("Read Distribution", indents)
               + readDistBuilder->toString(indents + 1)
               + indented_title("Insert Distribution", indents)
               + insertDistBuilder->toString(indents + 1)
               + indented_title("Remove Distribution", indents)
               + removeDistBuilder->toString(indents + 1)
               + indented_title("Read Data Map", indents)
               + readDataMapBuilder->toString(indents + 1)
               + indented_title("Remove Data Map", indents)
               + removeDataMapBuilder->toString(indents + 1);
    }

    ~LeafsHandshakeArgsGeneratorBuilder() override {
        delete readDistBuilder;
        delete insertDistBuilder;
        delete removeDistBuilder;
//        delete dataMapBuilder; //TODO may delete twice
    };

};

#endif //SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H
