package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;
import contention.benchmark.workload.distributions.abstractions.MutableDistribution;

import java.util.Random;
import java.util.concurrent.atomic.AtomicInteger;

public class CreakersAndWaveArgsGenerator implements ArgsGenerator {
    private final double creakersRatio;
    private final int creakersBegin;
    private final AtomicInteger waveBegin;
    private final AtomicInteger waveEnd;
    private final DataMap data;
    private final Distribution creakersDist;
    private final MutableDistribution waveDist;
    private final Random random;

    public CreakersAndWaveArgsGenerator(double creakersRatio, int creakersBegin,
                                        AtomicInteger waveBegin, AtomicInteger waveEnd,
                                        Distribution creakersDistribution, MutableDistribution waveDistribution,
                                        DataMap dataMap) {
        this.data = dataMap;
        this.creakersRatio = creakersRatio;
        this.creakersBegin = creakersBegin;
        this.waveBegin = waveBegin;
        this.waveEnd = waveEnd;
        this.random = new Random();
        this.creakersDist = creakersDistribution;
        this.waveDist = waveDistribution;
    }

    /**
     *
     * @return true - creakers, false - wave
     */
    private boolean nextCoin() {
        return random.nextDouble() < creakersRatio;
    }

    @Override
    public int nextGet() {
        int value;
        if (nextCoin()) {
            value = data.get(creakersBegin + creakersDist.next());
        } else {
            /**
             * In waveDist the first indexes have a higher probability
             */
            int localWaveBegin = waveBegin.get();
            int localWaveEnd = waveEnd.get();
            int localWaveLength =
                    (localWaveEnd - localWaveBegin + creakersBegin) % creakersBegin;
            int index = (localWaveBegin + waveDist.next(localWaveLength)) % creakersBegin;
            value = data.get(index);
        }
        return value;
    }

    private int waveShift(AtomicInteger waveEdge) {
        int localWaveEdge = waveEdge.get();
        int newWaveEdge;
        if (localWaveEdge == 0) {
            newWaveEdge = creakersBegin - 1;
        } else {
            newWaveEdge = localWaveEdge - 1;
        }
        waveEdge.weakCompareAndSet(localWaveEdge, newWaveEdge);
        return data.get(newWaveEdge);
    }

    @Override
    public int nextInsert() {
        return waveShift(waveBegin);
    }

    @Override
    public int nextRemove() {
        return waveShift(waveEnd);
    }

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }
}
