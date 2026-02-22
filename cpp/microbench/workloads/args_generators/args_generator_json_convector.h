//
// Created by Ravil Galiev on 27.07.2023.
//
#pragma once

#include "nlohmann/json.hpp"
#include "args_generator_builder.h"
#include "workloads/args_generators/impls/default_args_generator_builder.h"
#include "workloads/args_generators/impls/skewed_sets_args_generator_builder.h"
#include "workloads/args_generators/impls/skewed_insert_args_generator_builder.h"
#include "workloads/args_generators/impls/creakers_and_wave_args_generator_builder.h"
#include "workloads/args_generators/impls/temporary_skewed_args_generator_builder.h"
#include "workloads/args_generators/impls/leafs_handshake_args_generator_builder.h"
#include "workloads/args_generators/impls/generalized_args_generator_builder.h"
#include "workloads/args_generators/impls/null_args_generator_builder.h"
#include "workloads/args_generators/impls/range_query_args_generator_builder.h"
#include "errors.h"

namespace microbench::workload {

ArgsGeneratorBuilder* get_args_generator_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    ArgsGeneratorBuilder* args_generator_builder;
    if (class_name == "DefaultArgsGeneratorBuilder") {
        args_generator_builder = new DefaultArgsGeneratorBuilder();
    } else if (class_name == "SkewedSetsArgsGeneratorBuilder") {
        args_generator_builder = new SkewedSetsArgsGeneratorBuilder();
    } else if (class_name == "TemporarySkewedArgsGeneratorBuilder") {
        args_generator_builder = new TemporarySkewedArgsGeneratorBuilder();
    } else if (class_name == "CreakersAndWaveArgsGeneratorBuilder") {
        args_generator_builder = new CreakersAndWaveArgsGeneratorBuilder();
    } else if (class_name == "CreakersAndWavePrefillArgsGeneratorBuilder") {
        args_generator_builder = new CreakersAndWavePrefillArgsGeneratorBuilder();
    } else if (class_name == "LeafsHandshakeArgsGeneratorBuilder") {
        args_generator_builder = new LeafsHandshakeArgsGeneratorBuilder();
    } else if (class_name == "SkewedInsertArgsGeneratorBuilder") {
        args_generator_builder = new SkewedInsertArgsGeneratorBuilder();
    } else if (class_name == "GeneralizedArgsGeneratorBuilder") {
        args_generator_builder = new GeneralizedArgsGeneratorBuilder();
    } else if (class_name == "NullArgsGeneratorBuilder") {
        args_generator_builder = new NullArgsGeneratorBuilder();
    } else if (class_name == "RangeQueryArgsGeneratorBuilder") {
        args_generator_builder = new RangeQueryArgsGeneratorBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + class_name)
    }

    args_generator_builder->from_json(j);
    return args_generator_builder;
}

}  // namespace microbench::workload
