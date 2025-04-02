package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.IdDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.Distribution;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;

public class DefaultArgsGenerator implements ArgsGenerator {
    private final DataMap data;
    private final Distribution distribution;

    public DefaultArgsGenerator(DataMap data, Distribution distribution) {
        this.data = data;
        this.distribution = distribution;
    }

    private int next() {
        int index = distribution.next();
        return data.get(index);
    }

    @Override
    public int nextGet() {
        return next();
    }

    @Override
    public int nextInsert() {
        return next();
    }

    @Override
    public int nextRemove() {
        return next();
    }


    //TODO: maybe like this?
    public static class Builder implements ArgsGeneratorBuilder {
        public DistributionBuilder distributionBuilder = new UniformDistributionBuilder();
        public DataMapBuilder dataMapBuilder = new IdDataMapBuilder();
        transient public int range;

        public Builder setDistributionBuilder(DistributionBuilder distributionBuilder) {
            this.distributionBuilder = distributionBuilder;
            return this;
        }

        public Builder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
            this.dataMapBuilder = dataMapBuilder;
            return this;
        }

        @Override
        public Builder init(int range) {
            this.range = range;
//            dataMapBuilder.init(range);
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
        public StringBuilder toStringBuilder() {
            return null;
        }

        @Override
        public StringBuilder toStringBuilder(int indents) {
            return null;
        }

    }

}
