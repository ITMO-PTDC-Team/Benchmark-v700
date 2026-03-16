//
// Created by Ravil Galiev on 27.07.2023.
//
#pragma once

#include <memory>
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

ArgsGeneratorBuilderPtr get_args_generator_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    ArgsGeneratorBuilderPtr args_generator_builder;
    if (class_name == "DefaultArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<DefaultArgsGeneratorBuilder>();
    } else if (class_name == "SkewedSetsArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<SkewedSetsArgsGeneratorBuilder>();
    } else if (class_name == "TemporarySkewedArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<TemporarySkewedArgsGeneratorBuilder>();
    } else if (class_name == "CreakersAndWaveArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<CreakersAndWaveArgsGeneratorBuilder>();
    } else if (class_name == "CreakersAndWavePrefillArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<CreakersAndWavePrefillArgsGeneratorBuilder>();
    } else if (class_name == "LeafsHandshakeArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<LeafsHandshakeArgsGeneratorBuilder>();
    } else if (class_name == "SkewedInsertArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<SkewedInsertArgsGeneratorBuilder>();
    } else if (class_name == "GeneralizedArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<GeneralizedArgsGeneratorBuilder>();
    } else if (class_name == "NullArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<NullArgsGeneratorBuilder>();
    } else if (class_name == "RangeQueryArgsGeneratorBuilder") {
        args_generator_builder = std::make_unique<RangeQueryArgsGeneratorBuilder>();
    } else {
        setbench_error("JSON PARSER: Unknown class name ArgsGeneratorBuilder -- " + class_name)
    }

    args_generator_builder->from_json(j);
    return args_generator_builder;
}

}  // namespace microbench::workload
