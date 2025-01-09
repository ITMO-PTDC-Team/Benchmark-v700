package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.SkewedInsertArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class SkewedInsertArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public DistributionBuilder distributionBuilder = new UniformDistributionBuilder();

    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();

    public double skewedSize = 0;

    transient public int skewedLength;
    transient public int range;

    public SkewedInsertArgsGeneratorBuilder setDistributionBuilder(DistributionBuilder distributionBuilder) {
        this.distributionBuilder = distributionBuilder;
        return this;
    }

    public SkewedInsertArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    public SkewedInsertArgsGeneratorBuilder setSkewedSize(double skewedSize) {
        this.skewedSize = skewedSize;
        return this;
    }

    @Override
    public SkewedInsertArgsGeneratorBuilder init(int range) {
        this.range = range;
        // dataMapBuilder.init(range);
        skewedLength = (int) (range * skewedSize);
        return this;
    }

    @Override
    public SkewedInsertArgsGenerator build() {
        return new SkewedInsertArgsGenerator(skewedLength,
                distributionBuilder.build(range - skewedLength), dataMapBuilder.build());
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "SKEWED_INSERT", indents))
                .append(indentedTitleWithData("Skewed size", skewedSize, indents))
                .append(indentedTitle("Distribution", indents))
                .append(distributionBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("DataMap", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }

}
