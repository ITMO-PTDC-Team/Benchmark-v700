//
// Created by Ravil Galiev on 27.07.2023.
//
#pragma once

#include "json/single_include/nlohmann/json.hpp"
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

ArgsGeneratorBuilder* getArgsGeneratorFromJson(const nlohmann::json& j) {
    std::string className = j["ClassName"];
    ArgsGeneratorBuilder* argsGeneratorBuilder;
    if (className == "DefaultArgsGeneratorBuilder") {
        argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();
    } else if (className == "SkewedSetsArgsGeneratorBuilder") {
        argsGeneratorBuilder = new SkewedSetsArgsGeneratorBuilder();
    } else if (className == "TemporarySkewedArgsGeneratorBuilder") {
        argsGeneratorBuilder = new TemporarySkewedArgsGeneratorBuilder();
    } else if (className == "CreakersAndWaveArgsGeneratorBuilder") {
        argsGeneratorBuilder = new CreakersAndWaveArgsGeneratorBuilder();
    } else if (className == "CreakersAndWavePrefillArgsGeneratorBuilder") {
        argsGeneratorBuilder = new CreakersAndWavePrefillArgsGeneratorBuilder();
    } else if (className == "LeafsHandshakeArgsGeneratorBuilder") {
        argsGeneratorBuilder = new LeafsHandshakeArgsGeneratorBuilder();
    } else if (className == "SkewedInsertArgsGeneratorBuilder") {
        argsGeneratorBuilder = new SkewedInsertArgsGeneratorBuilder();
    } else if (className == "GeneralizedArgsGeneratorBuilder") {
        argsGeneratorBuilder = new GeneralizedArgsGeneratorBuilder();
    } else if (className == "NullArgsGeneratorBuilder") {
        argsGeneratorBuilder = new NullArgsGeneratorBuilder();
    } else if (className == "RangeQueryArgsGeneratorBuilder") {
        argsGeneratorBuilder = new RangeQueryArgsGeneratorBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + className)
    }

    argsGeneratorBuilder->fromJson(j);
    return argsGeneratorBuilder;
}
