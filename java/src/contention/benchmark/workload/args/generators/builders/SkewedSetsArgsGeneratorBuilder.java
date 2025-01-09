package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.SkewedSetsArgsGenerator;
import contention.benchmark.workload.distributions.builders.SkewedUniformDistributionBuilder;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class SkewedSetsArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public SkewedUniformDistributionBuilder readDistBuilder = new SkewedUniformDistributionBuilder();
    public SkewedUniformDistributionBuilder writeDistBuilder = new SkewedUniformDistributionBuilder();

    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();

    public double intersection = 0;

    transient public int writeSetBegins;
    transient public int range;

    public SkewedSetsArgsGeneratorBuilder setReadDistBuilder(SkewedUniformDistributionBuilder readDistBuilder) {
        this.readDistBuilder = readDistBuilder;
        return this;
    }

    public SkewedSetsArgsGeneratorBuilder setWriteDistBuilder(SkewedUniformDistributionBuilder writeDistBuilder) {
        this.writeDistBuilder = writeDistBuilder;
        return this;
    }

    public SkewedSetsArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    public SkewedSetsArgsGeneratorBuilder setIntersection(double intersection) {
        this.intersection = intersection;
        return this;
    }

    @Override
    public SkewedSetsArgsGeneratorBuilder init(int range) {
        this.range = range;
        // dataMapBuilder.init(range);
        writeSetBegins = readDistBuilder.getHotLength(range) - (int) (range * intersection);
        return this;
    }

    @Override
    public SkewedSetsArgsGenerator build() {
        return new SkewedSetsArgsGenerator(range, writeSetBegins, dataMapBuilder.build(),
                readDistBuilder.build(range), writeDistBuilder.build(range));
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "SKEWED_SETS", indents))
                .append(indentedTitleWithData("Intersection", intersection, indents))
                .append(indentedTitle("Read Distribution", indents))
                .append(readDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Write Distribution", indents))
                .append(writeDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("DataMap", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }

}
