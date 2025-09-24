package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.MutableDistribution;

import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

public class LeafsExtensionHandshakeArgsGenerator implements ArgsGenerator {
    private final DataMap readData;
    private final DataMap eraseData;

    private final MutableDistribution readDistribution;
    private final MutableDistribution insertDistribution;
    private final MutableDistribution removeDistribution;
    private final Random random;
    private final int range;

    private final AtomicInteger deletedValue;
    private final AtomicInteger curRange;

    public int center;


    public LeafsExtensionHandshakeArgsGenerator(int range,
                                                AtomicInteger deletedValue,
                                                AtomicInteger curRange,
                                                MutableDistribution readDistribution,
                                                MutableDistribution insertDistribution,
                                                MutableDistribution removeDistribution,
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
        this.curRange = curRange;
        this.center = range / 2;
    }

    @Override
    public int nextGet() {
        int localCurRange = curRange.get();
        int index = readDistribution.next(localCurRange);
        int rawValue = readData == null ? index : readData.get(index);

        return center - localCurRange / 2 + rawValue;
    }

    @Override
    public int nextInsert() {
        int localDeletedValue = deletedValue.get();

        int value;

        boolean isRight = random.nextDouble() >= 0.5;

        if (localDeletedValue == 0 || (isRight && localDeletedValue != range - 1)) {
            value = localDeletedValue + insertDistribution.next(range - 1 - localDeletedValue) + 1;
        } else {
            value = localDeletedValue - insertDistribution.next(localDeletedValue) - 1;
        }

        curRange.incrementAndGet();
        return value;
    }

    @Override
    public int nextRemove() {
        int localDeletedValue = deletedValue.get();
        int localCurRange = curRange.get();

        int index = removeDistribution.next(localCurRange);
        int rawValue = eraseData == null ? index : eraseData.get(index);
        int value = center - localCurRange / 2 + rawValue;

        //todo learn the difference between all kinds of weakCompareAndSet
        deletedValue.weakCompareAndSet(localDeletedValue, value);

        return value;
    }

    public int nextPrefill() {
        curRange.incrementAndGet();
        return nextGet();
    }

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }

}
