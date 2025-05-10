package contention.benchmark.workload.args.generators.builders;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.ReadWriteArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.IdDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;

public class ReadWriteArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public DistributionBuilder readDistributionBuilder = new UniformDistributionBuilder();
    public DistributionBuilder writeDistributionBuilder = new UniformDistributionBuilder();
    public DataMapBuilder dataMapBuilder = new IdDataMapBuilder();
    transient public int range;
//
//    public ReadWriteArgsGeneratorBuilder setDistributionBuilder(DistributionBuilder distributionBuilder) {
//        this.distributionBuilder = distributionBuilder;
//        return this;
//    }

    public ReadWriteArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    @Override
    public ReadWriteArgsGeneratorBuilder init(int range) {
        this.range = range;
        // dataMapBuilder.init(range);
        return this;
    }

    @Override
    public ReadWriteArgsGenerator build() {
        return new ReadWriteArgsGenerator(
                dataMapBuilder.build(),
                readDistributionBuilder.build(range),
                writeDistributionBuilder.build(range)
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "Read Write", indents))
                .append(indentedTitle("Read Distribution", indents))
                .append(readDistributionBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Write Distribution", indents))
                .append(writeDistributionBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("DataMap", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }

}
