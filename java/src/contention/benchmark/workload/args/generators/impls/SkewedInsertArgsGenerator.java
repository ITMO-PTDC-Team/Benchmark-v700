package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.tools.Pair;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;

public class SkewedInsertArgsGenerator implements ArgsGenerator {
    private final int skewedLength;
    private int insertedNumber;
    private final DataMap data;
    private final Distribution distribution;

    public SkewedInsertArgsGenerator(int skewedLength, Distribution distribution, DataMap data) {
        this.data = data;
        this.skewedLength = skewedLength;
        this.insertedNumber = 0;
        this.distribution = distribution;
    }

    @Override
    public int nextGet() {
        throw new UnsupportedOperationException("Get not supported");
    }

    @Override
    public int nextInsert() {
        int key;
        if (insertedNumber < skewedLength) {
            key = data.get(insertedNumber++);
        } else {
            key = data.get(skewedLength + distribution.next());
        }
        return key;
    }

    @Override
    public int nextRemove() {
        throw new UnsupportedOperationException("Remove not supported");
    }

    @Override
    public Pair<Integer, Integer> nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }
}
