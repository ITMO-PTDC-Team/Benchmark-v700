package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;
import contention.benchmark.workload.distributions.abstractions.MutableDistribution;

import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

public class LeafsHandshakeArgsGenerator implements ArgsGenerator {
    private final DataMap readData;
    private final DataMap eraseData;
    private final Distribution readDistribution;
    private final MutableDistribution insertDistribution;
    private final Distribution removeDistribution;
    private final Random random;
    private final int range;

    private final AtomicInteger deletedValue;


    public LeafsHandshakeArgsGenerator(int range,
                                       AtomicInteger deletedValue,
                                       Distribution readDistribution,
                                       MutableDistribution insertDistribution,
                                       Distribution removeDistribution,
                                       DataMap readData,
                                       DataMap eraseData) {
        this.readData = readData;
        this.eraseData = eraseData;
        this.readDistribution = readDistribution;
        this.insertDistribution = insertDistribution;
        this.removeDistribution = removeDistribution;
        this.random = new Random();
        this.range = range;
        this.deletedValue = deletedValue;
    }

    @Override
    public int nextGet() {
        int index = readDistribution.next();
        return readData.get(index);
    }

    @Override
    public int nextInsert() {
        int localDeletedValue = deletedValue.get();

        int value;

        boolean isRight = random.nextDouble() >= 0.5;

        if (localDeletedValue == 1 || (isRight && localDeletedValue != range)) {
            value = localDeletedValue + insertDistribution.next(range - 1 - localDeletedValue) + 1;
        } else {
            value = localDeletedValue - insertDistribution.next(localDeletedValue) - 1;
        }

        return value;
    }

    @Override
    public int nextRemove() {
        int localDeletedValue = deletedValue.get();
        int index = removeDistribution.next();
        int value = eraseData.get(index);

        //todo learn the difference between all kinds of weakCompareAndSet
        deletedValue.weakCompareAndSet(localDeletedValue, value);

        return value;
    }

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }
}
