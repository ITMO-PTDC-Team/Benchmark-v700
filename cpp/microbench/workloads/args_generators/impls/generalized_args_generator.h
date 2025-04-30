//
// Created by Ravil Galiev on 30.08.2022.
//

#ifndef SETBENCH_GENERALIZED_ARGS_GENERATOR_H
#define SETBENCH_GENERALIZED_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

template<typename K>
class GeneralizedArgsGenerator : public ArgsGenerator<K> {
private:
    ArgsGenerator<K> * _getGenerator;
    ArgsGenerator<K> * _insertGenerator;
    ArgsGenerator<K> * _removeGenerator;
    ArgsGenerator<K> * _rangeGenerator;

public:
    GeneralizedArgsGenerator(ArgsGenerator<K> * getGenerator, 
                             ArgsGenerator<K> * insertGenerator,
                             ArgsGenerator<K> * removeGenerator,
                             ArgsGenerator<K> * rangeGenerator)
            : _getGenerator(getGenerator), _insertGenerator(insertGenerator), _removeGenerator(removeGenerator), _rangeGenerator(rangeGenerator) {}


    K nextGet() {
        return _getGenerator->nextGet();
    }

    K nextInsert() {
        return _insertGenerator->nextInsert();
    }

    K nextRemove() {
        return _removeGenerator->nextRemove();
    }

    std::pair<K, K> nextRange() {
        return _rangeGenerator->nextRange();
    }

    ~GeneralizedArgsGenerator() {
        delete _getGenerator;
        delete _insertGenerator;
        delete _removeGenerator;
        delete _rangeGenerator;
    }
};


#include "workloads/distributions/distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "workloads/args_generators/args_generator_json_convector.h"
#include "globals_extern.h"

//template<typename K>
class GeneralizedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t _range;
    std::shared_ptr<ArgsGeneratorBuilder> getArgsGeneratorBuilder;
    std::shared_ptr<ArgsGeneratorBuilder> insertArgsGeneratorBuilder;
    std::shared_ptr<ArgsGeneratorBuilder> removeArgsGeneratorBuilder;
    std::shared_ptr<ArgsGeneratorBuilder> rangeQueryArgsGeneratorBuilder;
public:

    GeneralizedArgsGeneratorBuilder *init(size_t range) override {
        _range = _range;
        return this;
    }

    GeneralizedArgsGenerator<K> *build(Random64 &_rng) override {
        return new GeneralizedArgsGenerator<K>(
                                            getArgsGeneratorBuilder->build(_rng),
                                            insertArgsGeneratorBuilder->build(_rng),
                                            removeArgsGeneratorBuilder->build(_rng),
                                            rangeQueryArgsGeneratorBuilder->build(_rng));
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "GeneralizedArgsGeneratorBuilder";
    }

    void fromJson(const nlohmann::json &j) override;

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "Generalized", indents);
        return res;
    }

    ~GeneralizedArgsGeneratorBuilder() override {
        getArgsGeneratorBuilder.reset();
        insertArgsGeneratorBuilder.reset();
        removeArgsGeneratorBuilder.reset();
        rangeQueryArgsGeneratorBuilder.reset();
    };

};


#endif //SETBENCH_GENERALIZED_ARGS_GENERATOR_H
