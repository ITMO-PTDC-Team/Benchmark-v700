package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.IdDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.MutableDistributionBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.LeafsExtensionHandshakeArgsGenerator;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.distributions.builders.ZipfianDistributionBuilder;

import java.util.concurrent.atomic.AtomicInteger;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class LeafsExtensionHandshakeArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public MutableDistributionBuilder readDistBuilder = new UniformDistributionBuilder();
    public MutableDistributionBuilder insertDistBuilder = new ZipfianDistributionBuilder();
    public MutableDistributionBuilder removeDistBuilder = new UniformDistributionBuilder();

    public DataMapBuilder readDataMapBuilder = new IdDataMapBuilder();
    public DataMapBuilder removeDataMapBuilder = new IdDataMapBuilder();
    transient public AtomicInteger deletedValue;

    transient public AtomicInteger curRange;

    transient public int range;

    public LeafsExtensionHandshakeArgsGeneratorBuilder setReadDistBuilder(MutableDistributionBuilder readDistBuilder) {
        this.readDistBuilder = readDistBuilder;
        return this;
    }

    public LeafsExtensionHandshakeArgsGeneratorBuilder setInsertDistBuilder(MutableDistributionBuilder insertDistBuilder) {
        this.insertDistBuilder = insertDistBuilder;
        return this;
    }

    public LeafsExtensionHandshakeArgsGeneratorBuilder setRemoveDistBuilder(MutableDistributionBuilder removeDistBuilder) {
        this.removeDistBuilder = removeDistBuilder;
        return this;
    }

    public LeafsExtensionHandshakeArgsGeneratorBuilder setReadDataMapBuilder(DataMapBuilder readDataMapBuilder) {
        this.readDataMapBuilder = readDataMapBuilder;
        return this;
    }

    public LeafsExtensionHandshakeArgsGeneratorBuilder setEraseDataMapBuilder(DataMapBuilder removeDataMapBuilder) {
        this.removeDataMapBuilder = removeDataMapBuilder;
        return this;
    }

    @Override
    public LeafsExtensionHandshakeArgsGeneratorBuilder init(int range) {
        this.range = range;
        deletedValue = new AtomicInteger(range / 2);
        curRange = new AtomicInteger(10);//Math.max(10, parameters.size));
        return this;
    }

    @Override
    public LeafsExtensionHandshakeArgsGenerator build() {
        return new LeafsExtensionHandshakeArgsGenerator(range, deletedValue, curRange,
                readDistBuilder.build(),
                insertDistBuilder.build(),
                removeDistBuilder.build(),
                readDataMapBuilder.build(),
                removeDataMapBuilder.build()
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "LEAFS_EXTENSION_HANDSHAKE", indents))
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
