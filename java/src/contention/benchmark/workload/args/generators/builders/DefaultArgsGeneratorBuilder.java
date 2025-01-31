package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.impls.DefaultArgsGenerator;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.IdDataMapBuilder;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class DefaultArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public DistributionBuilder distributionBuilder = new UniformDistributionBuilder();
    public DataMapBuilder dataMapBuilder = new IdDataMapBuilder();
    transient public int range;

    public DefaultArgsGeneratorBuilder setDistributionBuilder(DistributionBuilder distributionBuilder) {
        this.distributionBuilder = distributionBuilder;
        return this;
    }

    public DefaultArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    @Override
    public DefaultArgsGeneratorBuilder init(int range) {
        this.range = range;
//        dataMapBuilder.init(range);
        return this;
    }

    @Override
    public DefaultArgsGenerator build() {
        return new DefaultArgsGenerator(
                dataMapBuilder.build(),
                distributionBuilder.build(range)
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "Default", indents))
                .append(indentedTitle("Distribution", indents))
                .append(distributionBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("DataMap", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }

}
