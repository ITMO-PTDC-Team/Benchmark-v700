package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;

public class SkewedSetsArgsGenerator implements ArgsGenerator {
    private final int range;
    private final int writeSetBegins;
    private final DataMap data;
    private final Distribution readDistribution;
    private final Distribution writeDistribution;

    public SkewedSetsArgsGenerator(int range, int writeSetBegins, DataMap data,
                                   Distribution readDistribution, Distribution writeDistribution) {
        this.data = data;
        this.range = range;
        this.writeSetBegins = writeSetBegins;
        this.readDistribution = readDistribution;
        this.writeDistribution = writeDistribution;
    }

    private int nextWrite() {
        int index = writeSetBegins + writeDistribution.next();
        if (index >= range) {
            index -= range;
        }
        return data.get(index);
    }

    @Override
    public int nextGet() {
        return data.get(readDistribution.next());
    }

    @Override
    public int nextInsert() {
        return nextWrite();
    }

    @Override
    public int nextRemove() {
        return nextWrite();
    }

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }
}
