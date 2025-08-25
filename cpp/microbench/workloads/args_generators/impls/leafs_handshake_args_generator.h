//
// Created by Ravil Galiev on 20.09.2023.
//

#ifndef SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H
#define SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H

#include <memory>
#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

template<typename K>
class LeafsHandshakeArgsGenerator : public ArgsGenerator<K> {
    size_t range;

    std::shared_ptr<Distribution> readDistribution;
    std::shared_ptr<MutableDistribution> insertDistribution;
    std::shared_ptr<Distribution> removeDistribution;
    Random64 &rng;
    PAD;
    std::atomic<size_t> *deletedValue;
    PAD;

    std::shared_ptr<DataMap<K>> readData;
    std::shared_ptr<DataMap<K>> removeData;

public:
    LeafsHandshakeArgsGenerator(Random64 &rng, size_t range, std::atomic<size_t> *deletedValue,
                                std::shared_ptr<Distribution> readDistribution, std::shared_ptr<MutableDistribution> insertDistribution,
                                std::shared_ptr<Distribution> removeDistribution, std::shared_ptr<DataMap<K>> readData, std::shared_ptr<DataMap<K>> removeData) :
            range(range),
            readDistribution(readDistribution),
            insertDistribution(insertDistribution),
            removeDistribution(removeDistribution),
            rng(rng), deletedValue(deletedValue),
            readData(readData),
            removeData(removeData) {}

    K nextGet() {
        return readData->get(readDistribution->next());
    }

    K nextInsert() {
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

    K nextRemove() {
        size_t localDeletedValue = *deletedValue;
        size_t value = removeData->get(removeDistribution->next());

        //todo learn the difference between all kinds of weakCompareAndSet
        deletedValue->compare_exchange_weak(localDeletedValue, value);

        return value;
    }

    std::pair<K, K> nextRange() {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~LeafsHandshakeArgsGenerator() = default;
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
class LeafsHandshakeArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t range;

    std::shared_ptr<DistributionBuilder> readDistBuilder = std::make_shared<UniformDistributionBuilder>();
    std::shared_ptr<MutableDistributionBuilder> insertDistBuilder = std::make_shared<ZipfianDistributionBuilder>();
    std::shared_ptr<DistributionBuilder> removeDistBuilder = std::make_shared<UniformDistributionBuilder>();

    std::shared_ptr<DataMapBuilder> readDataMapBuilder = std::make_shared<IdDataMapBuilder>();
    std::shared_ptr<DataMapBuilder> removeDataMapBuilder = std::make_shared<IdDataMapBuilder>();
    std::atomic<size_t> *deletedValue;

public:

    LeafsHandshakeArgsGeneratorBuilder *setReadDistBuilder(std::shared_ptr<DistributionBuilder> _readDistBuilder) {
        readDistBuilder = _readDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setInsertDistBuilder(std::shared_ptr<MutableDistributionBuilder> _insertDistBuilder) {
        insertDistBuilder = _insertDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setRemoveDistBuilder(std::shared_ptr<DistributionBuilder> _removeDistBuilder) {
        removeDistBuilder = _removeDistBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setReadDataMapBuilder(std::shared_ptr<DataMapBuilder> _readDataMapBuilder) {
        readDataMapBuilder = _readDataMapBuilder;
        return this;
    }

    LeafsHandshakeArgsGeneratorBuilder *setRemoveDataMapBuilder(std::shared_ptr<DataMapBuilder> _removeDataMapBuilder) {
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

    std::shared_ptr<ArgsGenerator<K>> build(Random64 &_rng) override {
        return std::shared_ptr<LeafsHandshakeArgsGenerator<K>>(new LeafsHandshakeArgsGenerator<K>(_rng, range, deletedValue,
                                                  readDistBuilder->build(_rng, range),
                                                  insertDistBuilder->build(_rng),
                                                  removeDistBuilder->build(_rng, range),
                                                  readDataMapBuilder->build(),
                                                  removeDataMapBuilder->build()));
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

    ~LeafsHandshakeArgsGeneratorBuilder() override = default;

};

#endif //SETBENCH_LEAFS_HANDSHAKE_ARGS_GENERATOR_H
