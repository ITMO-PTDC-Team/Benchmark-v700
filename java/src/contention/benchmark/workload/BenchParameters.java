package contention.benchmark.workload;

import contention.benchmark.json.serializers.DataMapBuilderSerializer;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.stop.condition.OperationCounter;
import contention.benchmark.workload.thread.loops.builders.PrefillInsertThreadLoopBuilder;

import static contention.benchmark.tools.StringFormat.getStringStage;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class BenchParameters {
    public int iterations = 1;
    public int range = 2048;
    public boolean detailedStats = false;
    public long afterPrefillDuration = 1000;
    public long afterWarmUpDuration = 0;
    public long betweenIterationsDuration = 100;


    public Parameters test;
    public Parameters prefill;
    public Parameters warmUp;

    public BenchParameters() {
        for (DataMapBuilder currentBuilder : DataMapBuilderSerializer.getBuilders().values()) {
            currentBuilder.init(range);
        }
        test = new Parameters();
        createDefaultPrefill();
        warmUp = new Parameters();
        // without warmUp
        warmUp.numThreads = 0;
    }

    public BenchParameters(Parameters test, Parameters prefill, Parameters warmUp) {
        this.test = test;
        this.prefill = prefill;
        this.warmUp = warmUp;
    }

    public BenchParameters createDefaultPrefill(int threadNum) {
        prefill = new Parameters()
                .setStopCondition(new OperationCounter(range / 2))
                .addThreadLoopBuilder(
                        new PrefillInsertThreadLoopBuilder(),
                        threadNum
                );
        return this;
    }

    public BenchParameters createDefaultPrefill() {
        return createDefaultPrefill(1);
    }

    public BenchParameters setIterations(int iterations) {
        this.iterations = iterations;
        return this;
    }

    public BenchParameters setRange(int range) {
        this.range = range;
        return this;
    }

    public BenchParameters setDetailedStats(boolean detailedStats) {
        this.detailedStats = detailedStats;
        return this;
    }

    public BenchParameters enableDetailedStats() {
        this.detailedStats = true;
        return this;
    }

    public BenchParameters disableDetailedStats() {
        this.detailedStats = false;
        return this;
    }


    public BenchParameters setAfterPrefillDuration(long afterPrefillDuration) {
        this.afterPrefillDuration = afterPrefillDuration;
        return this;
    }

    public BenchParameters setAfterWarmUpDuration(long afterWarmUpDuration) {
        this.afterWarmUpDuration = afterWarmUpDuration;
        return this;
    }

    public BenchParameters setBetweenIterationsDuration(long betweenIterationsDuration) {
        this.betweenIterationsDuration = betweenIterationsDuration;
        return this;
    }

    public BenchParameters setTest(Parameters test) {
        this.test = test;
        return this;
    }

    public BenchParameters setPrefill(Parameters prefill) {
        this.prefill = prefill;
        return this;
    }

    public BenchParameters setWarmUp(Parameters warmUp) {
        this.warmUp = warmUp;
        return this;
    }

    public void init() {
        for (DataMapBuilder currentBuilder : DataMapBuilderSerializer.getBuilders().values()) {
            currentBuilder.init(range);
        }
        test.init(range);
        prefill.init(range);
        warmUp.init(range);
    }

    public StringBuilder toStringBuilder() {
        return toStringBuilder(1);
    }

    public StringBuilder toStringBuilder(int indents) {
        StringBuilder result = new StringBuilder()
                .append(indentedTitleWithData("Range", range, indents))
                .append(indentedTitleWithData("Iterations", iterations, indents))
                .append(indentedTitleWithData("Detailed stats", this.detailedStats ? "enabled" : "disabled", indents))
                .append(indentedTitleWithData("After prefill duration", afterPrefillDuration, indents))
                .append(indentedTitleWithData("After warm up duration", afterWarmUpDuration, indents))
                .append(indentedTitleWithData("Between iterations duration", betweenIterationsDuration, indents));

        if (prefill.numThreads == 0) {
            result.append(getStringStage("Without prefill"));
        } else {
            result.append(getStringStage("Prefill parameters"))
                    .append(prefill.toStringBuilder(indents));
        }

        if (warmUp.numThreads == 0) {
            result.append(getStringStage("Without WarmUp"));
        } else {
            result.append(getStringStage("WarmUp parameters"))
                    .append(warmUp.toStringBuilder(indents));
        }

        result.append(getStringStage("Test parameters"))
                .append(test.toStringBuilder(indents));

        return result;
    }

}
