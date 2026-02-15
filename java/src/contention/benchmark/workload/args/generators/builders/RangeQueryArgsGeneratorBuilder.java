package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.RangeQueryArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.Distribution;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class RangeQueryArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    private int range;
    private int interval;
    private DistributionBuilder distributionBuilder;
    private DataMapBuilder dataMapBuilder;

    public RangeQueryArgsGeneratorBuilder setDistributionBuilder(DistributionBuilder distributionBuilder) {
        this.distributionBuilder = distributionBuilder;
        return this;
    }

    public RangeQueryArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    public RangeQueryArgsGeneratorBuilder setInterval(int interval) {
        this.interval = interval;
        return this;
    }

    @Override
    public ArgsGeneratorBuilder init(int range) {
        this.range = range;
        return this;
    }

    @Override
    public ArgsGenerator build() {
        Distribution distribution = distributionBuilder.build(range);
        DataMap dataMap = dataMapBuilder.build();
        return new RangeQueryArgsGenerator(dataMap, distribution, interval);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "RANGE_QUERY", indents))
                .append(indentedTitleWithData("Interval", String.valueOf(interval), indents))
                .append(indentedTitle("Distribution", indents))
                .append(distributionBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("DataMap", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }
}
