package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.LeafsHandshakeArgsGenerator;
import contention.benchmark.workload.data.map.builders.IdDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.abstractions.MutableDistributionBuilder;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.distributions.builders.ZipfianDistributionBuilder;

import java.util.concurrent.atomic.AtomicInteger;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class LeafsHandshakeArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public DistributionBuilder readDistBuilder = new UniformDistributionBuilder();
    public MutableDistributionBuilder insertDistBuilder = new ZipfianDistributionBuilder();
    public DistributionBuilder removeDistBuilder = new UniformDistributionBuilder();

    public DataMapBuilder readDataMapBuilder = new IdDataMapBuilder();
    public DataMapBuilder removeDataMapBuilder = new IdDataMapBuilder();
    transient public AtomicInteger deletedValue;

    transient public int range;

    public LeafsHandshakeArgsGeneratorBuilder setReadDistBuilder(DistributionBuilder readDistBuilder) {
        this.readDistBuilder = readDistBuilder;
        return this;
    }

    public LeafsHandshakeArgsGeneratorBuilder setInsertDistBuilder(MutableDistributionBuilder insertDistBuilder) {
        this.insertDistBuilder = insertDistBuilder;
        return this;
    }

    public LeafsHandshakeArgsGeneratorBuilder setRemoveDistBuilder(DistributionBuilder removeDistBuilder) {
        this.removeDistBuilder = removeDistBuilder;
        return this;
    }

    public LeafsHandshakeArgsGeneratorBuilder setReadDataMapBuilder(DataMapBuilder readDataMapBuilder) {
        this.readDataMapBuilder = readDataMapBuilder;
        return this;
    }

    public LeafsHandshakeArgsGeneratorBuilder setRemoveDataMapBuilder(DataMapBuilder removeDataMapBuilder) {
        this.removeDataMapBuilder = removeDataMapBuilder;
        return this;
    }

    @Override
    public LeafsHandshakeArgsGeneratorBuilder init(int range) {
        this.range = range;
        deletedValue = new AtomicInteger(range / 2);
        return this;
    }

    @Override
    public LeafsHandshakeArgsGenerator build() {
        return new LeafsHandshakeArgsGenerator(range, deletedValue,
                readDistBuilder.build(range),
                insertDistBuilder.build(),
                removeDistBuilder.build(range),
                readDataMapBuilder.build(),
                removeDataMapBuilder.build()
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "LEAFS_HANDSHAKE", indents))
                .append(indentedTitle("Read Distribution", indents))
                .append(readDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Insert Distribution", indents))
                .append(insertDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Remove Distribution", indents))
                .append(removeDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Read DataMap", indents))
                .append(readDataMapBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Remove DataMap", indents))
                .append(removeDataMapBuilder.toStringBuilder(indents + 1));
    }

}

