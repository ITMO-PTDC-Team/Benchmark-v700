//
// Created by Ravil Galiev on 25.07.2023.
//
#pragma once

#include <fstream>
#include <iostream>
#include "json/single_include/nlohmann/json.hpp"

#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/impls/generalized_args_generator.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/bench_parameters.h"

ArgsGeneratorBuilder* get_default_args_generator_builder() {
    return (new DefaultArgsGeneratorBuilder())
        ->set_distribution_builder((new ZipfianDistributionBuilder())->set_alpha(1.0))
        ->set_data_map_builder(new ArrayDataMapBuilder());
}

ArgsGeneratorBuilder* get_temporary_skewed_args_generator_builder() {
    return (new TemporarySkewedArgsGeneratorBuilder())
        ->set_set_number(5)
        ->set_hot_times(new int64_t[5]{1, 2, 3, 4, 5})
        ->set_relax_times(new int64_t[5]{1, 2, 3, 4, 5})
        ->set_hot_size_and_ratio(0, 0.1, 0.8)
        ->set_hot_size_and_ratio(1, 0.2, 0.7)
        ->set_hot_size_and_ratio(2, 0.3, 0.6)
        ->set_hot_size_and_ratio(3, 0.4, 0.6)
        ->set_hot_size_and_ratio(4, 0.5, 0.7)
        ->enable_manual_setting_set_begins()
        ->set_set_begins(new double[5]{0, 0.1, 0.2, 0.3, 0.05});
}

ArgsGeneratorBuilder* get_creakers_and_wave_args_generator_builder() {
    return (new CreakersAndWaveArgsGeneratorBuilder())
        ->set_creakers_ratio(0.2)
        ->set_wave_size(0.2)
        ->set_creakers_size(0.1)
        ->set_data_map_builder(new IdDataMapBuilder());
}

ArgsGeneratorBuilder* get_null_args_generator_builder() {
    return (new NullArgsGeneratorBuilder());
}

ArgsGeneratorBuilder* get_range_query_args_generator_builder() {
    return (new RangeQueryArgsGeneratorBuilder())
        ->set_distribution_builder((new ZipfianDistributionBuilder())->set_alpha(1.0))
        ->set_data_map_builder(new ArrayDataMapBuilder())
        ->set_interval(100);
}

ArgsGeneratorBuilder* get_generalized_args_generator_builder(ArgsGeneratorBuilder* inside) {
    // std::vector<std::string> operations{"get", "insert", "remove"};
    return (new GeneralizedArgsGeneratorBuilder())
        ->add_args_generator_builder({"get"}, inside)
        ->add_args_generator_builder({"insert"}, get_creakers_and_wave_args_generator_builder())
        ->add_args_generator_builder({"remove"}, get_default_args_generator_builder())
        ->add_args_generator_builder({"rangeQuery"}, get_range_query_args_generator_builder());
    // ->setDistributionBuilder((new ZipfianDistributionBuilder())->setAlpha(1.0))
    // ->setDataMapBuilder(new ArrayDataMapBuilder()));
}

ThreadLoopBuilder* get_default_thread_loop_builder(ArgsGeneratorBuilder* args_generator_builder) {
    return (new DefaultThreadLoopBuilder())
        ->set_ins_ratio(0.1)
        ->set_rem_ratio(0.1)
        ->set_rq_ratio(0)
        ->setArgsGeneratorBuilder(args_generator_builder);
}

ThreadLoopBuilder* get_temporary_operation_thread_loop_builder(
    ArgsGeneratorBuilder* args_generator_builder) {
    return (new TemporaryOperationsThreadLoopBuilder())
        ->set_stages_number(3)
        ->set_stages_durations(new size_t[3]{1000, 2000, 3000})
        ->set_ratios(0, new RatioThreadLoopParameters(0.1, 0.1, 0))
        ->set_ratios(1, new RatioThreadLoopParameters(0.2, 0.2, 0))
        ->set_ratios(2, new RatioThreadLoopParameters(0.3, 0.3, 0))
        ->set_args_generator_builder(args_generator_builder);
}

Parameters* get_creakers_and_wave_prefiller(size_t range,
                                        CreakersAndWaveArgsGeneratorBuilder* args_generator_builder) {
    CreakersAndWavePrefillArgsGeneratorBuilder* prefill_args_generator_builder =
        new CreakersAndWavePrefillArgsGeneratorBuilder(args_generator_builder);
    return (new Parameters())
        ->add_thread_loop_builder((new PrefillInsertThreadLoopBuilder())
                                   ->set_args_generator_builder(prefill_args_generator_builder))
        ->set_stop_condition(
            new OperationCounter(prefill_args_generator_builder->get_prefill_length(range)));
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

    Parameters* test = new Parameters();

    /**
     * We will need to set the stop condition and workloads.
     *
     * First, let's create a stop condition: Timer with 10 second (10000 millis).
     */

    StopCondition* stop_condition = new Timer(10000);

    /**
     * Setup a workload.
     */

    /**
     * in addition to the DefaultArgsGeneratorBuilder,
     * TemporarySkewedArgsGeneratorBuilder and CreakersAndWaveArgsGeneratorBuilder are also
     * presented in the corresponding functions
     */
    ArgsGeneratorBuilder* args_generator_builder = get_default_args_generator_builder();
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
    ThreadLoopBuilder* thread_loop_builder = get_default_thread_loop_builder(args_generator_builder);
    //                = getTemporaryOperationThreadLoopBuilder(argsGeneratorBuilder);

    /**
     * now add the ThreadLoopBuilders (you can add several different)
     * to the parameter class indicating the number of threads.
     * You can also optionally specify the cores to which threads should bind (-1 without binding).
     */
    test->add_thread_loop_builder(thread_loop_builder, 8, "~2.0.0.1-3.3")
        ->set_stop_condition(stop_condition);

    bench_parameters.set_test(test).create_default_prefill();
    //        .setPrefill(getCreakersAndWavePrefiller(
    //            2048, (CreakersAndWaveArgsGeneratorBuilder*)argsGeneratorBuilder));

    std::cout << "to json\n";

    nlohmann::json json = bench_parameters;

    std::ofstream out("example.json");

    out << json.dump(4);

    std::cout << "end\n";
}
