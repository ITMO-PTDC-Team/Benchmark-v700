[//]: # (# Java )

[//]: # (Java version Benchmark-v700 based on [Synchrobench]&#40;https://github.com/gramoli/synchrobench&#41;.)

[//]: # (Synchrobench is a micro-benchmark suite used to evaluate synchronization)

[//]: # (techniques on data structures. Synchrobench is written in C/java and Java and)

[//]: # (currently includes arrays, binary trees, hash tables, linked lists, queues and)

[//]: # (skip lists that are synchronized with copy-on-write, locks, read-modify-write,)

[//]: # (read-copy-update and transactional memory. A non-synchronized version of these)

[//]: # (data structures is proposed in each language as a baseline to measure the)

[//]: # (performance gain on multi-&#40;/many-&#41;core machines.)

## Data structures in Java

[//]: # (The Java version of synchrobench &#40;synchrobench-java&#41; provides variants of the)
[//]: # (algorithms presented in these papers:)

The Java version provides the same algorithm variants as those implemented in [Synchrobench](https://github.com/gramoli/synchrobench). 
They are presented in these papers:
- V. Aksenov, V. Gramoli, P. Kuznetsov, A. Malova, S. Ravi. A Concurrency-Optimal Binary Search Tree.
  In Euro-Par, 2017. arXiv:1705.02851
- V. Gramoli, P. Kuznetsov, S. Ravi, D. Shang. A Concurrency-Optimal List-Based Set. arXiv:1502.01633, 2015.
- K. Sagonas and K. Winblad. Contention Adapting Search Trees. In ISPDC 2015.
- V. Gramoli and R. Guerraoui. Reusable Concurrent Data Types. In ECOOP 2014.
- D. Drachsler, M. Vechev and E. Yahav. Practical concurrent binary search
  trees via logical ordering. In PPoPP, pages 343–356, 2014.
- V. Gramoli and R. Guerraoui. Democratizing Transactional Programming. CACM
  57(1):86-93, 2014.
- T. Crain, V. Gramoli and M. Raynal. A contention-friendly search tree. In
  Euro-Par, pages 229–240, 2013.
- T. Crain, V. Gramoli and M. Raynal. No hot spot non-blocking skip list. In
  ICDCS, 2013.
- T. Crain, V. Gramoli and M. Raynal. A contention-friendly methodology for
  search structures. Technical Report RR-1989, INRIA, 2012.
- F. Ellen, P. Fatourou, E. Ruppert and F. van Breugel. Non-blocking binary
  search trees. In PODC, pages 131–140, 2010.
- N. G. Bronson, J. Casper, H. Chafi and K. Olukotun. A practical
  concurrent binary search tree. In PPoPP, 2010.
- P. Felber, V. Gramoli and R. Guerraoui. Elastic Transactions. In DISC 2009.
- C. Click. A lock-free hash table. Personal communication. 2007.
- M. M. Michael and M. L. Scott. Simple, fast, and practical non-blocking and
  blocking concurrent queue algorithms. In PODC, 1996.  
  Please check the copyright notice of each implementation.

## Synchronizations

The Java-like algorithms are synchronized with copy-on-write wrappers,
read-modify-write using exclusively compare-and-swap, transactional memory
in their software forms using bytecode instrumentation, locks.

The transactional memory algorithm used here is E-STM presented in:
- P. Felber, V. Gramoli, and R. Guerraoui. Elastic transactions. In DISC, pages
  93–108, 2009.

Other Transactional Memory implemenations can be tested with Synchrobench
in Java, by adding the DeuceSTM-based libraries in the directory:
synchrobench-java/src/org/deuce/transaction/  
The transactional memories that were tested successfully with synchrobench in
Java are E-STM, LSA, PSTM, SwissTM and TL2 as presented in:
- P. Felber, V. Gramoli, and R. Guerraoui. Elastic transactions. In DISC, pages
  93–108, 2009.
- T. Riegel, P. Felber, and C. Fetzer. A lazy snapshot algorithm with eager
  validation. In DISC, 2006.
- V. Gramoli and R. Guerraoui. Reusable Concurrent Data Types. In ECOOP 2014
- A. Dragojevic, R. Guerraoui, M. Kapalka. Stretching transactional memory. In
  PLDI, p.155-165, 2009.
- D. Dice, O. Shalev and N. Shavit. Transactional locking II. In DISC, 2006.


[//]: # (# SETUP)

[//]: # (The project has the following structure:)

[//]: # (```shell)

[//]: # (./src)

[//]: # (├── arrays)

[//]: # (├── contention)

[//]: # (├── hashtables)

[//]: # (├── kotlin)

[//]: # (├── linkedlists)

[//]: # (├── org)

[//]: # (├── queues)

[//]: # (├── skiplists)

[//]: # (└── trees)

[//]: # (```)

[//]: # (The main folders to pay attention to are [ds]&#40;./ds&#41; and [microbench]&#40;./microbench/&#41;. )
[//]: # (The first one stands for data structures - all available for benchmarking data structures are stored there. )
[//]: # (The latter stands for launching benchmarks and there you can specify many arguments, )
[//]: # (in particular, [different workloads]&#40;./WORKLOADS.md&#41;.  )

##### NOTE
[//]: # (**The software design is described in [SOFTWARE_DESIGN]&#40;SOFTWARE_DESIGN.md&#41;.**  )
**The software design is described on the [main page](./../README.md#software-design).**  
**And how to add new workloads is described in [ADDING_NEW_WORKLOAD](ADDING_NEW_WORKLOAD.md).**



[//]: # (Before launching benchmarks for different data structures --- )
[//]: # (it's necessary to build the project, and it can be done with the following command:)

[//]: # (```shell)
[//]: # ()
[//]: # (cd setbench/microbench)
[//]: # (make -j)
[//]: # (```)


# LAUNCH

Build and launch occurs using Gradle.
Commands to launch have the following pattern:

```shell
./gradlew run --args='<bench_args>'
```

## Benchmark arguments

+ `-data-structure <data_structure_class_name>` or `-ds <data_structure_class_name>` — data structure class name;
+ `-json-file <file_name>` — file with launch parameters in the json format
  ([BenchParameters](src/contention/benchmark/workload/BenchParameters.java), [example](src/contention/benchmark/json/JsonExample.java));
+ `-result-file <file_name>` — file to output the results in the json format (optional);
+ `-iter <number of iterations>` — the number of benchmarking (default 1).

Benchmarking stages can also be specified separately:

+ `-range` — key range;
+ `-prefill <file_name>` — file with prefill stage parameters in json format;
+ `-test <file_name>` — file with test stage parameters in json format;
+ `-warm-up <file_name>` — file with warm up stage parameters in json format;
+ `-create-default-prefill` — create a default prefill (fill the data structure in half) (optional).

#### Example:
```shell
./gradlew run --args='-ds skiplists.lockfree.NonBlockingFriendlySkipListMap -json-file example.json -result-file result.json'
```

# Configuring Launch Parameters

[Example of configuring launch parameters](src/contention/benchmark/json/JsonExample.java).
Use `./gradlew toJson` to run.

The first step is to create the [BenchParameters class](src/contention/benchmark/workload/BenchParameters.java).
The class stores the range of keys and parameters each of epoch: prefill, warm up and test.

```java
    BenchParameters benchParameters = new BenchParameters();
```

Set the range of keys.

```java
    benchParameters.setRange(2048);
```

Create the [Parameters class](src/contention/benchmark/workload/Parameters.java) for benchmarking (test).

```java
    Parameters test = new Parameters();
```

Now, we need to set the [stop condition](src/contention/benchmark/workload/stop/condition/StopCondition.java) and workloads.

First, let's create and set a stop condition: [Timer](src/contention/benchmark/workload/stop/condition/Timer.java) with 10 second (10000 millis).

```java
    StopCondition stopCondition = new Timer(10000);
    test.setStopCondition(stopCondition);
```

The next step is to configure the workload.

The workload consists of 4 types of entities:
+ Distribution — a distribution of a random variable
+ DataMap — for converting a distribution's output into a key
+ ArgsGenerator — creates operands for an operation
+ ThreadLoop — the logic for interacting with a data structure.

There are builders to create each type of entity:
[ThreadLoopBuilder](src/contention/benchmark/workload/thread/loops/abstractions/ThreadLoopBuilder.java),
[ArgsGeneratorBuilder](src/contention/benchmark/workload/args/generators/abstractions/ArgsGeneratorBuilder.java),
[DistributionBuilder](src/contention/benchmark/workload/distributions/abstractions/DistributionBuilder.java),
[DataMapBuilder](src/contention/benchmark/workload/data/map/abstractions/DataMapBuilder.java).

[//]: # (**The software design is described in [SOFTWARE_DESIGN]&#40;SOFTWARE_DESIGN.md&#41;.**  )
**The software design is described on the [main page](./../README.md#software-design).**  
**And how to add new workloads is described in [ADDING_NEW_WORKLOAD](ADDING_NEW_WORKLOAD.md).**

Let's create a standard workload with Zipfian distribution.

At first, we create the DistributionBuilder and DataMapBuilder.
```java
    DistributionBuilder distributionBuilder = new ZipfianDistributionBuilder().setAlpha(1.0);

    DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();
```

The next step is to create the ArgsGeneratorBuilder.
```java
    ArgsGeneratorBuilder argsGeneratorBuilder =
            new DefaultArgsGeneratorBuilder()
                    .setDistributionBuilder(distributionBuilder)
                    .setDataMapBuilder(dataMapBuilder);
```

The last step is to create the ThreadLoopBuilder.
We create a DefaultThreadLoop with the probability 0.1 (10%) of calling the insertion and remove operation
and set our ArgsGeneratorBuilder.
```java
    ThreadLoopBuilder threadLoopBuilder =
            new DefaultThreadLoopBuilder()
                    .setInsRatio(0.1)
                    .setRemRatio(0.1)
                    .setArgsGeneratorBuilder(argsGeneratorBuilder);
```

Now set the ThreadLoop class to Parameters with the number of threads with this load.

[//]: # (Also, as the third parameter, you can specify the cores to which threads should bind &#40;-1 without binding&#41;.)
[//]: # (In our case, the first two threads will not be bound to any core, the 3-th and 4-th threads will bound to the fisrt core, )
[//]: # (the 5-th thread will bound to the second core, and so on)
[//]: # (&#40;The first CPU on the system corresponds to a cpu value of 0, the next CPU corresponds to a cpu value of 1, and so on.&#41;.)
```java
    test.addThreadLoopBuilder(threadLoopBuilder, 8);
```

Specify the test parameters in the benchParameters and create a default prefill (fill the data structure in half):
```java
    benchParameters.setTest(test).createDefaultPrefill();
```

Convert parameters to json format and output:
```java
    String json = JsonConverter.toJson(benchParameters);

    try (PrintWriter out = new PrintWriter("example.json", StandardCharsets.UTF_8)) {
        out.print(json);
        System.out.println("Successfully written data to the file");
    } catch (IOException e) {
        e.printStackTrace();
    }
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
