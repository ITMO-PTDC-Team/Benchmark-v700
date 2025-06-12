# SETUP

[//]: # (First of all, it's necessary to clone the repository with submodules:)

[//]: # (```shell)
[//]: # (git clone https://gitlab.com/mr_ravil/setbench.git --recurse-submodules)
[//]: # (```)

```shell
git clone https://github.com/Mr-Ravil/Benchmark-v700.git --recurse-submodules
```

The project has the following structure:
```shell
.
├── common
├── docker
├── ds
├── json
├── lib
├── microbench
├── microbench_experiments
└── tools
```

The main folders to pay attention to are [ds](ds) and [microbench](microbench/).  
The first folder is for data structures. It contains all the data structures available for benchmarking
that were implemented in [SetBench](https://gitlab.com/trbot86/setbench).  
The second folder contains the benchmark with various workloads.

[//]: # (The first one stands for data structures - all available for benchmarking data structures are stored there  )

[//]: # (The latter stands for launching benchmarks and there you can specify many arguments.)

[//]: # (in particular, [different workloads]&#40;WORKLOADS.md&#41;.  )

##### NOTE 
**The software design is described on the [main page](./../README.md#software-design).**  
**And how to add new workloads is described in [ADDING_NEW_WORKLOAD](ADDING_NEW_WORKLOAD.md).**



Before launching benchmarks for different data structures —
it's necessary to build the project, and it can be done with the following command:

```shell
cd setbench/microbench
make -j
```

[//]: # (You can read about the redis implementation in [REDIS]&#40;gsat/REDIS.md&#41; )

[//]: # (and about other tree implementations in [TREES_IMPLEMENTATION]&#40;gsat/TREES_IMPLEMENTATION.md&#41;.)


# LAUNCH

After setting up the project, you can launch benchmarks. Commands to launch have the following pattern:

```shell
cd setbench/microbench

<memory allocator> ./bin/<data_structure_name>.debra <bench_args>
```

[//]: # (FROM SETBENCH WIKI: )
You can use `LD_PRELOAD` to load different memory allocators.
(Using a scalable allocator is crucial in evaluating concurrent data structures.)
SetBench includes JEMalloc, TCMalloc, Hoard, Supermalloc and Mimalloc libraries in /lib.
(Recently, my recommendations have been to use JEMalloc or Mimalloc---whichever performs better. :))

[//]: # (For instance, you can plug JEMalloc into the above example, instead of the default allocator, by running:)

**NOTE: I now STRONGLY recommend using mimalloc https://github.com/microsoft/mimalloc 
instead of jemalloc for all of your experiments in concurrent data structures!!**

#### Example:
```shell
LD_PRELOAD=../lib/libmimalloc.so ./bin/aksenov_splaylist_64.debra -json-file json_example/example.json -result-file json_example/result.json 
```

## Benchmark arguments

+ `-json-file <file_name>` — file with launch parameters in the json format ([BenchParameters](microbench/workloads/bench_parameters.h), [example](microbench/json_example/json_example.cpp));
+ `-result-file <file_name>` — file to output the results in the json format (optional).

Benchmarking parameters can also be specified separately
(a new `BenchParameters` will be created with the specified parameters)
or additionally 
(the parameters specified in `-json-file` will be overwritten):

+ `-range` — key range;
+ `-prefill <file_name>` — file with prefill stage parameters in json format;
+ `-test <file_name>` — file with test stage parameters in json format;
+ `-warm-up <file_name>` — file with warm up stage parameters in json format;
+ `-create-default-prefill` — create a default prefill: fill the data structure in half 
(ignored if `-prefill` argument was already specified).


# Configuring Launch Parameters

[Example of configuring launch parameters](microbench/json_example/json_example.cpp).
Use [Makefile](microbench/json_example/Makefile) to compile.

The first step is to create the [BenchParameters class](microbench/workloads/bench_parameters.h).
The class stores the range of keys and parameters each of epoch: prefill, warm up and test.

```c++
    BenchParameters benchParameters;
```

Set the range of keys.

```c++
    benchParameters.setRange(2048);
```

Create the [Parameters class](microbench/workloads/parameters.h) for benchmarking (test).

```c++
    Parameters *test = new Parameters();
```

Now, we need to set the [stop condition](microbench/workloads/stop_condition/stop_condition.h) and workloads.

First, let's create and set a stop condition: [Timer](microbench/workloads/stop_condition/impls/timer.h) with 10 second (10000 millis).

```c++
    StopCondition *stopCondition = new Timer(10000);
    test.setStopCondition(stopCondition);
```

The next step is to configure the workload.

The workload consists of 4 types of entities:
+ Distribution — a distribution of a random variable
+ DataMap — for converting a distribution's output into a key
+ ArgsGenerator — creates operands for an operation
+ ThreadLoop — the logic for interacting with a data structure.

There are builders to create each type of entity:
[ThreadLoopBuilder](microbench/workloads/thread_loops/thread_loop_builder.h),
[ArgsGeneratorBuilder](microbench/workloads/args_generators/args_generator_builder.h),
[DistributionBuilder](microbench/workloads/distributions/distribution_builder.h),
[DataMapBuilder](microbench/workloads/data_maps/data_map_builder.h).

[//]: # (**The software design is described in [SOFTWARE_DESIGN]&#40;SOFTWARE_DESIGN.md&#41;.**  )
**The software design is described on the [main page](./../README.md#software-design).**  
**And how to add new workloads is described in [ADDING_NEW_WORKLOAD](ADDING_NEW_WORKLOAD.md).**

[//]: # (**How to add new workloads is described [here]&#40;./ADDING_NEW_WORKLOAD.md&#41;.**)

Let's create a standard workload with Zipf distribution.

At first, we create the DistributionBuilder and DataMapBuilder.
```c++
    DistributionBuilder *distributionBuilder 
            = (new ZipfianDistributionBuilder())
                    ->setAlpha(1.0);

    DataMapBuilder *dataMapBuilder 
            = new ArrayDataMapBuilder();
```

The next step is to create the ArgsGeneratorBuilder.
```c++
    ArgsGeneratorBuilder *argsGeneratorBuilder
            = (new DefaultArgsGeneratorBuilder())
                    ->setDistributionBuilder(distributionBuilder)
                    ->setDataMapBuilder(dataMapBuilder);
```

The last step is to create the ThreadLoopBuilder. 
We create a DefaultThreadLoop with the probability 0.1 (10%) of calling the insertion and remove operation
and set our ArgsGeneratorBuilder.
```c++
    ThreadLoopBuilder *threadLoopBuilder
            = (new DefaultThreadLoopBuilder())
                    ->setInsRatio(0.1)
                    ->setRemRatio(0.1)
                    ->setArgsGeneratorBuilder(argsGeneratorBuilder);
```

Now set the ThreadLoop class to Parameters with the number of threads with this load.

Also, as the third parameter, you can specify the cores to which threads should bind (-1 without binding).
In our case, the first two threads will not be bound to any core, the 3-th and 4-th threads will bound to the fisrt core, 
the 5-th thread will bound to the second core, and so on
(The first CPU on the system corresponds to a cpu value of 0, the next CPU corresponds to a cpu value of 1, and so on.).
```c++
    test->addThreadLoopBuilder(
            threadLoopBuilder, 8,
            new int[8]{-1, -1, 0, 0, 1, 2, 3, 3}
    );
```

Specify the test parameters in the benchParameters and create a default prefill (fill the data structure in half):
```c++
    benchParameters.setTest(test).createDefaultPrefill();
```

Convert parameters to json format and output:
```c++
    nlohmann::json json = benchParameters;

    std::ofstream out("example.json");

    out << json.dump(4);
```


[//]: # (# Troubleshooting)
[//]: # ()
[//]: # (If something breaks after the launch, or there is such a problem:)
[//]: # ()
[//]: # (```shell)
[//]: # (PAPI ERROR: thread 0 unable to add event PAPI_L2_DCM: Permission level does not permit operation)
[//]: # (```)
[//]: # (then the following can help:)
[//]: # ()
[//]: # (```shell)
[//]: # (sudo sysctl kernel.perf_event_paranoid=1)
[//]: # (```)


