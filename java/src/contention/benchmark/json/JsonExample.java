package contention.benchmark.json;

import contention.benchmark.workload.BenchParameters;
import contention.benchmark.workload.Parameters;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.builders.*;
import contention.benchmark.workload.data.map.builders.*;
import contention.benchmark.workload.distributions.builders.*;
import contention.benchmark.workload.thread.loops.builders.*;
import contention.benchmark.workload.stop.condition.StopCondition;
import contention.benchmark.workload.stop.condition.Timer;
import contention.benchmark.workload.thread.loops.abstractions.ThreadLoopBuilder;
import contention.benchmark.workload.thread.loops.parameters.RatioThreadLoopParameters;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;

public class JsonExample {
    public static ArgsGeneratorBuilder getDefaultArgsGeneratorBuilder() {
        return new DefaultArgsGeneratorBuilder()
                .setDistributionBuilder(
                        new ZipfianDistributionBuilder()
                                .setAlpha(1.0)
                )
                .setDataMapBuilder(
                        new ArrayDataMapBuilder()
                );
    }

    public static void makeSyntheticBinaryData() {
        try {
            FileOutputStream out = new FileOutputStream("test-binary-file");
            byte []arr = {0, 0, 0, 1};
            for (int i = 0; i<2048; ++i) {
                out.write(arr);
            }
            out.close();
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }

    public static ArgsGeneratorBuilder getFileBasedArgsGeneratorBuilder() {
        // Generate file first
        makeSyntheticBinaryData();

        return new DefaultArgsGeneratorBuilder()
                .setDistributionBuilder(
                        new UniformDistributionBuilder()
                )
                .setDataMapBuilder(
                        new BinaryFileDataMapBuilder()
                                .setFilename(new File("test-binary-file").getAbsolutePath())
                                .setShuffleFlag(false)
                );
    }

    public static ArgsGeneratorBuilder getTemporarySkewedArgsGeneratorBuilder() {
        return new TemporarySkewedArgsGeneratorBuilder()
                .setSetNumber(5)
                .setHotTimes(new int[]{1, 2, 3, 4, 5})
                .setRelaxTimes(new int[]{1, 2, 3, 4, 5})
                .setHotSizeAndRatio(0, 0.1, 0.8)
                .setHotSizeAndRatio(1, 0.2, 0.7)
                .setHotSizeAndRatio(2, 0.3, 0.6)
                .setHotSizeAndRatio(3, 0.4, 0.6)
                .setHotSizeAndRatio(4, 0.5, 0.7);
    }

    public static ArgsGeneratorBuilder getCreakersAndWaveArgsGeneratorBuilder() {
        return new CreakersAndWaveArgsGeneratorBuilder()
                .setCreakersRatio(0.2)
                .setWaveSize(0.2)
                .setCreakersSize(0.1)
                .setDataMapBuilder(new ArrayDataMapBuilder());
    }


    public static ThreadLoopBuilder getDefaultThreadLoopBuilder(ArgsGeneratorBuilder argsGeneratorBuilder) {
        return new DefaultThreadLoopBuilder()
                .setInsertRatio(0.1)
                .setRemoveRatio(0.1)
                .setArgsGeneratorBuilder(argsGeneratorBuilder);
    }

    public static ThreadLoopBuilder getTemporaryOperationThreadLoopBuilder(ArgsGeneratorBuilder argsGeneratorBuilder) {
        return new TemporaryOperationsThreadLoopBuilder()
                .setStagesNumber(3)
                .setStagesDurations(new int[]{1000, 2000, 3000})
                .setRatios(0, new RatioThreadLoopParameters(0.1, 0.1))
                .setRatios(1, new RatioThreadLoopParameters(0.2, 0.2))
                .setRatios(2, new RatioThreadLoopParameters(0.3, 0.3))
                .setArgsGeneratorBuilder(argsGeneratorBuilder);
    }

    public static void main(String[] args) {
        /**
         * The first step is the creation the BenchParameters class.
         */
        BenchParameters benchParameters = new BenchParameters();

        /**
         * Set the range of keys.
         */

        benchParameters.setRange(2048);

        /**
         * Create the Parameters class for benchmarking (test).
         */

        Parameters test = new Parameters();

        /**
         * We will need to set the stop condition and workloads.
         *
         * First, let's create a stop condition: Timer with 10 second (10000 millis).
         */

        StopCondition stopCondition = new Timer(10000);
        test.setStopCondition(stopCondition);

        /**
         * Setup a workload.
         */

        /**
         * in addition to the DefaultArgsGeneratorBuilder,
         * TemporarySkewedArgsGeneratorBuilder and CreakersAndWaveArgsGeneratorBuilder are also presented
         * in the corresponding functions
         */
        ArgsGeneratorBuilder argsGeneratorBuilder = getFileBasedArgsGeneratorBuilder();
        //ArgsGeneratorBuilder argsGeneratorBuilder = getDefaultArgsGeneratorBuilder();

        /**
         * in addition to the DefaultThreadLoopBuilder,
         * TemporaryOperationThreadLoopBuilder is also presented in the corresponding function
         */
        ThreadLoopBuilder threadLoopBuilder = getDefaultThreadLoopBuilder(argsGeneratorBuilder);

        /**
         * now add the ThreadLoopBuilders (you can add several different)
         * to the parameter class indicating the number of threads.
         * You can also optionally specify the cores to which threads should bind (-1 without binding).
         */
        test.addThreadLoopBuilder(threadLoopBuilder, 8)
                .setStopCondition(stopCondition);


        benchParameters.setTest(test)
                .createDefaultPrefill();

        benchParameters.setDetailedStats(true);

        String json = JsonConverter.toJson(benchParameters);

        try (PrintWriter out = new PrintWriter("example.json", StandardCharsets.UTF_8)) {
            out.print(json);
            System.out.println("Successfully written data to the file");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}

