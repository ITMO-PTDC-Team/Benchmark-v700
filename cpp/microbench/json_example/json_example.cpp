//
// Created by Ravil Galiev on 25.07.2023.
//

#define DISTR_CONV_IMPL

#include <fstream>
#include <iostream>
#include <memory>
#include "args_generators/args_generator_builder.h"
#include "args_generators/impls/creakers_and_wave_args_generator_builder.h"
#include "args_generators/impls/temporary_skewed_args_generator_builder.h"
#include "data_maps/builders/array_data_map_builder.h"
#include "data_maps/builders/id_data_map_builder.h"

#include "parameters.h"
#include "stop_condition/impls/operation_counter.h"
#include "thread_loops/impls/prefill_insert_thread_loop.h"
#include "workloads/args_generators/impls/generalized_args_generator_builder.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/bench_parameters.h"

using namespace microbench::workload;

ArgsGeneratorBuilderPtr get_default_args_generator_builder() {
    auto zipf = std::make_unique<ZipfianDistributionBuilder>();
    zipf->set_alpha(1.0);

    auto builder = std::make_unique<DefaultArgsGeneratorBuilder>();
    builder->set_distribution_builder(std::move(zipf))
        .set_data_map_builder(std::make_shared<ArrayDataMapBuilder>());
    return builder;
}

ArgsGeneratorBuilderPtr get_temporary_skewed_args_generator_builder() {
    auto builder = std::make_unique<TemporarySkewedArgsGeneratorBuilder>();
    builder->set_set_number(5)
        .set_hot_times({1, 2, 3, 4, 5})
        .set_relax_times({1, 2, 3, 4, 5})
        .set_hot_size_and_ratio(0, 0.1, 0.8)
        .set_hot_size_and_ratio(1, 0.2, 0.7)
        .set_hot_size_and_ratio(2, 0.3, 0.6)
        .set_hot_size_and_ratio(3, 0.4, 0.6)
        .set_hot_size_and_ratio(4, 0.5, 0.7)
        .enable_manual_setting_set_begins()
        .set_set_begins({0, 0.1, 0.2, 0.3, 0.05});
    return builder;
}

ArgsGeneratorBuilderPtr get_creakers_and_wave_args_generator_builder() {
    auto builder = std::make_unique<CreakersAndWaveArgsGeneratorBuilder>();
    builder->set_creakers_ratio(0.2).set_wave_size(0.3).set_creakers_size(0.2).set_data_map_builder(
        std::make_shared<IdDataMapBuilder>());
    return builder;
}

ArgsGeneratorBuilderPtr get_null_args_generator_builder() {
    return std::make_unique<NullArgsGeneratorBuilder>();
}

ArgsGeneratorBuilderPtr get_range_query_args_generator_builder() {
    auto zipf = std::make_unique<ZipfianDistributionBuilder>();
    zipf->set_alpha(1.0);
    auto builder = std::make_unique<RangeQueryArgsGeneratorBuilder>();
    builder->set_distribution_builder(std::move(zipf))
        .set_data_map_builder(std::make_shared<ArrayDataMapBuilder>())
        .set_interval(100);
    return builder;
}

ArgsGeneratorBuilderPtr get_generalized_args_generator_builder(ArgsGeneratorBuilderPtr inside) {
    auto builder = std::make_unique<GeneralizedArgsGeneratorBuilder>();
    builder->add_args_generator_builder({"get"}, std::move(inside))
        .add_args_generator_builder({"insert"}, get_creakers_and_wave_args_generator_builder())
        .add_args_generator_builder({"remove"}, get_default_args_generator_builder())
        .add_args_generator_builder({"rangeQuery"}, get_range_query_args_generator_builder());
    return builder;
}

ThreadLoopBuilderPtr get_default_thread_loop_builder(
    ArgsGeneratorBuilderPtr args_generator_builder) {
    auto builder = std::make_unique<DefaultThreadLoopBuilder>();
    builder->set_ins_ratio(0.1).set_rem_ratio(0.1).set_rq_ratio(0).set_args_generator_builder(
        std::move(args_generator_builder));
    return builder;
}

ThreadLoopBuilderPtr get_temporary_operation_thread_loop_builder(
    ArgsGeneratorBuilderPtr args_generator_builder) {
    auto builder = std::make_unique<TemporaryOperationsThreadLoopBuilder>();
    builder->set_stages_number(3)
        .set_stages_durations({1000, 2000, 3000})
        .set_ratios(0, RatioThreadLoopParameters(0.1, 0.1, 0))
        .set_ratios(1, RatioThreadLoopParameters(0.2, 0.2, 0))
        .set_ratios(2, RatioThreadLoopParameters(0.3, 0.3, 0))
        .set_args_generator_builder(std::move(args_generator_builder));
    return builder;
}

Parameters get_creakers_and_wave_prefiller(
    size_t range, std::unique_ptr<CreakersAndWaveArgsGeneratorBuilder> args_generator_builder) {
    auto prefill_args_generator_builder =
        std::make_unique<CreakersAndWavePrefillArgsGeneratorBuilder>();
    prefill_args_generator_builder->set_parameters_by_builder(*args_generator_builder);
    auto lim = prefill_args_generator_builder->get_prefill_length();
    auto prefill_insert_builder = std::make_unique<PrefillInsertThreadLoopBuilder>();
    prefill_insert_builder->set_args_generator_builder(std::move(prefill_args_generator_builder));
    return Parameters()
        .add_thread_loop_builder(std::move(prefill_insert_builder))
        .set_stop_condition(std::make_shared<OperationCounter>(lim));
}

int main() {
    /**
     * The first step is the creation the BenchParameters class.
     */

    BenchParameters bench_parameters;

    /**
     * Set the range of keys.
     */

    bench_parameters.set_range(2048);

    /**
     * Create the Parameters class for benchmarking (test).
     */

    Parameters test = Parameters();

    /**
     * We will need to set the stop condition and workloads.
     *
     * First, let's create a stop condition: Timer with 10 second (10000 millis).
     */

    StopConditionPtr stop_condition = std::make_shared<Timer>(10000);

    /**
     * Setup a workload.
     */

    /**
     * in addition to the DefaultArgsGeneratorBuilder,
     * TemporarySkewedArgsGeneratorBuilder and CreakersAndWaveArgsGeneratorBuilder are also
     * presented in the corresponding functions
     */
    ArgsGeneratorBuilderPtr args_generator_builder = get_default_args_generator_builder();
    //               = getCreakersAndWaveArgsGeneratorBuilder();
    //                = getTemporarySkewedArgsGeneratorBuilder();
    /*
     * Use this argsGeneratorBuilder for Generalized Testing
     */
    //    ArgsGeneratorBuilder*  actualArgsGeneratorBuilde =
    //    getGeneralizedArgsGeneratorBuilder(argsGeneratorBuilder);
    /**
     * in addition to the DefaultThreadLoopBuilder,
     * TemporaryOperationThreadLoopBuilder is also presented in the corresponding function
     */
    ThreadLoopBuilderPtr thread_loop_builder =
        get_default_thread_loop_builder(std::move(args_generator_builder));
    //                = getTemporaryOperationThreadLoopBuilder(argsGeneratorBuilder);

    /**
     * now add the ThreadLoopBuilders (you can add several different)
     * to the parameter class indicating the number of threads.
     * You can also optionally specify the cores to which threads should bind (-1 without binding).
     */
    test.add_thread_loop_builder(std::move(thread_loop_builder), 8, "~2.0.0.1-3.3")
        .set_stop_condition(std::move(stop_condition));

    bench_parameters.set_test(test).create_default_prefill();
    //        .setPrefill(getCreakersAndWavePrefiller(
    //            2048, (CreakersAndWaveArgsGeneratorBuilder*)argsGeneratorBuilder));

    std::cout << "to json\n";

    nlohmann::json json = bench_parameters;

    std::ofstream out("example.json");

    out << json.dump(4);

    std::cout << "end\n";
}
