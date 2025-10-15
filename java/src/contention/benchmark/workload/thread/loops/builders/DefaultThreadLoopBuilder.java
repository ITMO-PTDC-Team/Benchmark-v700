package contention.benchmark.workload.thread.loops.builders;

import contention.abstractions.DataStructure;
import contention.benchmark.workload.stop.condition.StopCondition;
import contention.benchmark.workload.args.generators.builders.DefaultArgsGeneratorBuilder;
import contention.benchmark.workload.thread.loops.abstractions.ThreadLoopBuilder;
import contention.benchmark.workload.thread.loops.impls.DefaultThreadLoop;
import contention.benchmark.workload.thread.loops.parameters.RatioThreadLoopParameters;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;

import java.lang.reflect.Method;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class DefaultThreadLoopBuilder extends ThreadLoopBuilder {
    public RatioThreadLoopParameters parameters = new RatioThreadLoopParameters();

    public ArgsGeneratorBuilder argsGeneratorBuilder = new DefaultArgsGeneratorBuilder();


    public DefaultThreadLoopBuilder setWriteRatio(double writeRatio) {
        parameters.setWriteRatio(writeRatio);
        return this;
    }

    public DefaultThreadLoopBuilder setInsertRatio(double insertRatio) {
        parameters.insertRatio = insertRatio;
        return this;
    }

    public DefaultThreadLoopBuilder setRangeQueryRatio(double rqRatio) {
        parameters.rangeQueryRatio = rqRatio;
        return this;
    }

    public DefaultThreadLoopBuilder setRemoveRatio(double removeRatio) {
        parameters.removeRatio = removeRatio;
        return this;
    }

    public DefaultThreadLoopBuilder setWriteAllsRatio(double writeAllsRatio) {
        parameters.writeAllsRatio = writeAllsRatio;
        return this;
    }

    public DefaultThreadLoopBuilder setSnapshotsRatio(double snapshotsRatio) {
        parameters.snapshotsRatio = snapshotsRatio;
        return this;
    }

    public DefaultThreadLoopBuilder setArgsGeneratorBuilder(ArgsGeneratorBuilder argsGeneratorBuilder) {
        this.argsGeneratorBuilder = argsGeneratorBuilder;
        return this;
    }

    @Override
    public DefaultThreadLoopBuilder init(int range) {
        argsGeneratorBuilder.init(range);
        return this;
    }

    @Override
    public DefaultThreadLoop build(int threadId, DataStructure<Integer> dataStructure,
                                   Method[] methods, StopCondition stopCondition) {
        return new DefaultThreadLoop(threadId, dataStructure, methods, stopCondition,
                parameters, argsGeneratorBuilder.build());
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "Default", indents))
                .append(parameters.toStringBuilder(indents))
                .append(indentedTitle("ArgsGenerator", indents))
                .append(argsGeneratorBuilder.toStringBuilder(indents + 1));
    }
}
