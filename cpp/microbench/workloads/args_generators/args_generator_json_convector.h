//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
#define SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H


#include "json/single_include/nlohmann/json.hpp"
#include "args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/impls/skewed_sets_args_generator.h"
#include "workloads/args_generators/impls/skewed_insert_args_generator.h"
#include "workloads/args_generators/impls/creakers_and_wave_args_generator.h"
#include "workloads/args_generators/impls/temporary_skewed_args_generator.h"
#include "workloads/args_generators/impls/leafs_handshake_args_generator.h"
#include "errors.h"

class BaseArgsGeneratorBuilderFactory {
    public:
        virtual ~BaseArgsGeneratorBuilderFactory() = default;
        virtual ArgsGeneratorBuilder* create() = 0;
};  
    
template <typename ArgsGeneratorBuilder>
class ArgsGeneratorBuilderFactory : public BaseArgsGeneratorBuilderFactory {
    public:
        ArgsGeneratorBuilder *create() override {
            return new ArgsGeneratorBuilder();
        }
};

inline static std::map<std::string, std::unique_ptr<BaseArgsGeneratorBuilderFactory>> argsGeneratorFactoryMap = [] {
    std::map<std::string, std::unique_ptr<BaseArgsGeneratorBuilderFactory>> map;
    map.insert({"DefaultArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<DefaultArgsGeneratorBuilder>>()});
    map.insert({"SkewedSetsArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<SkewedSetsArgsGeneratorBuilder>>()});
    map.insert({"TemporarySkewedArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<TemporarySkewedArgsGeneratorBuilder>>()});
    map.insert({"CreakersAndWaveArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<CreakersAndWaveArgsGeneratorBuilder>>()});
    map.insert({"CreakersAndWavePrefillArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<CreakersAndWavePrefillArgsGeneratorBuilder>>()});
    map.insert({"LeafsHandshakeArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<LeafsHandshakeArgsGeneratorBuilder>>()});
    map.insert({"SkewedInsertArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<SkewedInsertArgsGeneratorBuilder>>()});
    map.insert({"GeneralizedArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<GeneralizedArgsGeneratorBuilder>>()});
    map.insert({"NullArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<NullArgsGeneratorBuilder>>()}); 
    map.insert({"RangeQueryArgsGeneratorBuilder", std::make_unique<ArgsGeneratorBuilderFactory<RangeQueryArgsGeneratorBuilder>>()}); 
    return map;
}();

ArgsGeneratorBuilder *getArgsGeneratorFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    if (argsGeneratorFactoryMap.find(className) != argsGeneratorFactoryMap.end()) {
        ArgsGeneratorBuilder *argsGeneratorBuilder = argsGeneratorFactoryMap[className]->create();
        argsGeneratorBuilder->fromJson(j);
        return argsGeneratorBuilder;
    }
    setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + className);
}

#endif //SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
