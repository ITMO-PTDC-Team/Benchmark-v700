//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
#define SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H

#include <map>
#include "json/single_include/nlohmann/json.hpp"
#include "args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/impls/generalized_args_generator.h"
#include "workloads/args_generators/impls/skewed_sets_args_generator.h"
#include "workloads/args_generators/impls/skewed_insert_args_generator.h"
#include "workloads/args_generators/impls/creakers_and_wave_args_generator.h"
#include "workloads/args_generators/impls/temporary_skewed_args_generator.h"
#include "workloads/args_generators/impls/leafs_handshake_args_generator.h"
#include "workloads/args_generators/impls/null_args_generator.h"
#include "workloads/args_generators/impls/range_query_args_generator.h"
#include "errors.h"

class BaseArgsGeneratorBuilderFactory {
public:
    virtual ~BaseArgsGeneratorBuilderFactory() = default;
    virtual ArgsGeneratorBuilder* create() = 0;
};

template <typename ArgsGeneratorBuilder>
class ArgsGeneratorBuilderFactory : public BaseArgsGeneratorBuilderFactory{
    public:
        ArgsGeneratorBuilder *create() override {
            return new ArgsGeneratorBuilder();
        }
};

ArgsGeneratorBuilder *getArgsGeneratorFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    std::map<std::string, std::unique_ptr<BaseArgsGeneratorBuilderFactory>> factoryMap;
    factoryMap.insert({"DefaultArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<DefaultArgsGeneratorBuilder>>()});
    factoryMap.insert({"SkewedSetsArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<SkewedSetsArgsGeneratorBuilder>>()});
    factoryMap.insert({"TemporarySkewedArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<TemporarySkewedArgsGeneratorBuilder>>()});
    factoryMap.insert({"CreakersAndWaveArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<CreakersAndWaveArgsGeneratorBuilder>>()});
    factoryMap.insert({"CreakersAndWavePrefillArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<CreakersAndWavePrefillArgsGeneratorBuilder>>()});
    factoryMap.insert({"LeafsHandshakeArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<LeafsHandshakeArgsGeneratorBuilder>>()});
    factoryMap.insert({"SkewedInsertArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<SkewedInsertArgsGeneratorBuilder>>()});
    factoryMap.insert({"GeneralizedArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<GeneralizedArgsGeneratorBuilder>>()});
    factoryMap.insert({"NullArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<NullArgsGeneratorBuilder>>()}); 
    factoryMap.insert({"RangeQueryArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<RangeQueryArgsGeneratorBuilder>>()}); 
    if (factoryMap.find(className) != factoryMap.end()) {
        ArgsGeneratorBuilder *argsGeneratorBuilder = factoryMap[className]->create();
        argsGeneratorBuilder->fromJson(j);
        return argsGeneratorBuilder;
    }
    setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + className);
}

#include "workloads/args_generators/impls/generalized_args_generator_impl.h"

#endif //SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
