//
// Created by Ravil Galiev on 25.07.2023.
//

#ifndef SETBENCH_JSON_EXAMPLE_CPP
#define SETBENCH_JSON_EXAMPLE_CPP

#include <fstream>
#include <iostream>
#include "json/single_include/nlohmann/json.hpp"

#include "workloads/args_generators/impls/default_args_generator.h"
#include "workloads/args_generators/impls/generalized_args_generator.h"
#include "workloads/thread_loops/impls/default_thread_loop.h"
#include "workloads/bench_parameters.h"

#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"

typedef long long ll;

ArgsGeneratorBuilder* getDefaultArgsGeneratorBuilder() {
    return (new DefaultArgsGeneratorBuilder())
        ->setDistributionBuilder((new ZipfianDistributionBuilder())->setAlpha(1.0))
        ->setIndexMapBuilder(new ArrayIndexMapBuilder());
}

ArgsGeneratorBuilder* getTemporarySkewedArgsGeneratorBuilder() {
    return (new TemporarySkewedArgsGeneratorBuilder())
        ->setSetNumber(5)
        ->setHotTimes(new long long[5]{1, 2, 3, 4, 5})
        ->setRelaxTimes(new long long[5]{1, 2, 3, 4, 5})
        ->setHotSizeAndRatio(0, 0.1, 0.8)
        ->setHotSizeAndRatio(1, 0.2, 0.7)
        ->setHotSizeAndRatio(2, 0.3, 0.6)
        ->setHotSizeAndRatio(3, 0.4, 0.6)
        ->setHotSizeAndRatio(4, 0.5, 0.7)
        ->enableManualSettingSetBegins()
        ->setSetBegins(new double[5]{0, 0.1, 0.2, 0.3, 0.05});
}

ArgsGeneratorBuilder* getCreakersAndWaveArgsGeneratorBuilder() {
    return (new CreakersAndWaveArgsGeneratorBuilder())
        ->setCreakersRatio(0.2)
        ->setWaveSize(0.2)
        ->setCreakersSize(0.1)
        ->setIndexMapBuilder(new IdIndexMapBuilder());
}

ArgsGeneratorBuilder* getNullArgsGeneratorBuilder() {
    return (new NullArgsGeneratorBuilder());
}

ArgsGeneratorBuilder* getRangeQueryArgsGeneratorBuilder() {
    return (new RangeQueryArgsGeneratorBuilder())
    ->setDistributionBuilder((new ZipfianDistributionBuilder())->setAlpha(1.0))
    ->setIndexMapBuilder(new ArrayIndexMapBuilder())
    ->setInterval(100);
}

ArgsGeneratorBuilder* getGeneralizedArgsGeneratorBuilder(ArgsGeneratorBuilder* inside) {
    // std::vector<std::string> operations{"get", "insert", "remove"};
    return (new GeneralizedArgsGeneratorBuilder())
        ->addArgsGeneratorBuilder({"get"}, inside)
        ->addArgsGeneratorBuilder({"insert"}, getCreakersAndWaveArgsGeneratorBuilder())
        ->addArgsGeneratorBuilder({"remove"}, getDefaultArgsGeneratorBuilder())
        ->addArgsGeneratorBuilder({"rangeQuery"}, getRangeQueryArgsGeneratorBuilder());
        // ->setDistributionBuilder((new ZipfianDistributionBuilder())->setAlpha(1.0))
        // ->setIndexMapBuilder(new ArrayIndexMapBuilder()));
}

ThreadLoopBuilder* getDefaultThreadLoopBuilder(ArgsGeneratorBuilder* argsGeneratorBuilder) {
    return (new DefaultThreadLoopBuilder())
        ->setInsRatio(0.1)
        ->setRemRatio(0.1)
        ->setRqRatio(0)
        ->setArgsGeneratorBuilder(argsGeneratorBuilder);
}

ThreadLoopBuilder* getTemporaryOperationThreadLoopBuilder(ArgsGeneratorBuilder *argsGeneratorBuilder) {
    return (new TemporaryOperationsThreadLoopBuilder())
        ->setStagesNumber(3)
        ->setStagesDurations(new size_t[3]{1000, 2000, 3000})
        ->setRatios(0, new RatioThreadLoopParameters(0.1, 0.1, 0))
        ->setRatios(1, new RatioThreadLoopParameters(0.2, 0.2, 0))
        ->setRatios(2, new RatioThreadLoopParameters(0.3, 0.3, 0))
        ->setArgsGeneratorBuilder(argsGeneratorBuilder);
}

Parameters* getCreakersAndWavePrefiller(size_t range,
                                        CreakersAndWaveArgsGeneratorBuilder* argsGeneratorBuilder) {
    CreakersAndWavePrefillArgsGeneratorBuilder* prefillArgsGeneratorBuilder =
        new CreakersAndWavePrefillArgsGeneratorBuilder(argsGeneratorBuilder);
    return (new Parameters())
        ->addThreadLoopBuilder((new PrefillInsertThreadLoopBuilder())
                                   ->setArgsGeneratorBuilder(prefillArgsGeneratorBuilder))
        ->setStopCondition(
            new OperationCounter(prefillArgsGeneratorBuilder->getPrefillLength(range)));
}

int main() {
    /**
     * The first step is the creation the BenchParameters class.
     */

    BenchParameters benchParameters;

    /**
     * Set the range of keys.
     */

    benchParameters.setRange(2048);

    /**
     * Create the Parameters class for benchmarking (test).
     */

    Parameters* test = new Parameters();

    /**
     * We will need to set the stop condition and workloads.
     *
     * First, let's create a stop condition: Timer with 10 second (10000 millis).
     */

    StopCondition* stopCondition = new Timer(10000);

    /**
     * Setup a workload.
     */

    /**
     * in addition to the DefaultArgsGeneratorBuilder,
     * TemporarySkewedArgsGeneratorBuilder and CreakersAndWaveArgsGeneratorBuilder are also
     * presented in the corresponding functions
     */
    ArgsGeneratorBuilder* argsGeneratorBuilder
                = getDefaultArgsGeneratorBuilder();
//               = getCreakersAndWaveArgsGeneratorBuilder();
//                = getTemporarySkewedArgsGeneratorBuilder();
    /*
     * Use this argsGeneratorBuilder for Generalized Testing 
     */
//    ArgsGeneratorBuilder*  actualArgsGeneratorBuilde = getGeneralizedArgsGeneratorBuilder(argsGeneratorBuilder);
            /**
             * in addition to the DefaultThreadLoopBuilder,
             * TemporaryOperationThreadLoopBuilder is also presented in the corresponding function
             */
    ThreadLoopBuilder* threadLoopBuilder
                = getDefaultThreadLoopBuilder(argsGeneratorBuilder);
//                = getTemporaryOperationThreadLoopBuilder(argsGeneratorBuilder);

    /**
     * now add the ThreadLoopBuilders (you can add several different)
     * to the parameter class indicating the number of threads.
     * You can also optionally specify the cores to which threads should bind (-1 without binding).
     */
    test->addThreadLoopBuilder(threadLoopBuilder, 8, "~2.0.0.1-3.3")
            ->setStopCondition(stopCondition);

    benchParameters.setTest(test)
        .createDefaultPrefill();
    //        .setPrefill(getCreakersAndWavePrefiller(
    //            2048, (CreakersAndWaveArgsGeneratorBuilder*)argsGeneratorBuilder));

    std::cout << "to json\n";

    nlohmann::json json = benchParameters;

    std::ofstream out("example.json");

    out << json.dump(4);

    std::cout << "end\n";
}

#endif //SETBENCH_JSON_EXAMPLE_CPP
