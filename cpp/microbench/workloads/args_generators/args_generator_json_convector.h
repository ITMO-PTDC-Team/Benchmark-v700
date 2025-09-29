//
// Created by Ravil Galiev on 27.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
#define SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H


#include <nlohmann/json.hpp>
#include "args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/impls/skewed_sets_args_generator.h"
#include "workloads/args_generators/impls/skewed_insert_args_generator.h"
#include "workloads/args_generators/impls/creakers_and_wave_args_generator.h"
#include "workloads/args_generators/impls/temporary_skewed_args_generator.h"
#include "workloads/args_generators/impls/leafs_handshake_args_generator.h"
#include "workloads/args_generators/impls/generalized_args_generator.h"
#include "workloads/args_generators/impls/null_args_generator.h"
#include "workloads/args_generators/impls/range_query_args_generator.h"
#include "errors.h"

std::shared_ptr<ArgsGeneratorBuilder> getArgsGeneratorFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    std::shared_ptr<ArgsGeneratorBuilder> argsGeneratorBuilder;
    if (className == "DefaultArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<DefaultArgsGeneratorBuilder>();
    } else if (className == "SkewedSetsArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<SkewedSetsArgsGeneratorBuilder>();
    } else if (className == "TemporarySkewedArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<TemporarySkewedArgsGeneratorBuilder>();
    } else if (className == "CreakersAndWaveArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<CreakersAndWaveArgsGeneratorBuilder>();
    } else if (className == "CreakersAndWavePrefillArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<CreakersAndWavePrefillArgsGeneratorBuilder>();
    } else if (className == "LeafsHandshakeArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<LeafsHandshakeArgsGeneratorBuilder>();
    } else if (className == "SkewedInsertArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<SkewedInsertArgsGeneratorBuilder>();
    } else if (className == "GeneralizedArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<GeneralizedArgsGeneratorBuilder>();
    } else if (className == "NullArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<NullArgsGeneratorBuilder>();
    } else if (className == "RangeQueryArgsGeneratorBuilder") {
        argsGeneratorBuilder = std::make_shared<RangeQueryArgsGeneratorBuilder>();
    } else {
        setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + className)
    }

    argsGeneratorBuilder->fromJson(j);
    return argsGeneratorBuilder;
}


#endif //SETBENCH_ARGS_GENERATOR_JSON_CONVECTOR_H
