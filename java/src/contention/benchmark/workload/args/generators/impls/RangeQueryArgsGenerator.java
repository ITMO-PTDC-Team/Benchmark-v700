package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;

public class RangeQueryArgsGenerator implements ArgsGenerator {
    private final DataMap data;
    private final Distribution distribution;
    private final int interval;

    public RangeQueryArgsGenerator(DataMap data, Distribution distribution, int interval) {
        this.data = data;
        this.distribution = distribution;
        this.interval = interval;
    }

    @Override
    public int nextGet() {
        throw new UnsupportedOperationException("Get not supported");
    }

    @Override
    public int nextInsert() {
        throw new UnsupportedOperationException("Insert not supported");
    }

    @Override
    public int nextRemove() {
        throw new UnsupportedOperationException("Remove not supported");
    }

    @Override
    public int[] nextRange() {
        int index = distribution.next();
        int left = data.get(index);
        int right = data.get(index + interval);

        if (left > right) {
            return new int[]{right, left};
        }

        return new int[]{left, right};
    }
}
