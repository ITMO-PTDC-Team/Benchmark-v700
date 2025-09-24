package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.distributions.abstractions.Distribution;

public class TemporarySkewedArgsGenerator implements ArgsGenerator {
    private final DataMap data;

    private final Distribution[] hotDists;
    private final Distribution relaxDist;
    private long time;
    private int pointer;
    private boolean relaxTime;
    private final int range;
    private final int setNumber;
    private final int[] hotTimes;
    private final int[] relaxTimes;
    private final int[] setBegins;

    public TemporarySkewedArgsGenerator(int setNumber, int range, int[] hotTimes, int[] relaxTimes, int[] setBegins,
                                        Distribution[] hotDists, Distribution relaxDist, DataMap dataMap) {
        this.data = dataMap;
        this.hotDists = hotDists;
        this.relaxDist = relaxDist;
        this.time = 0;
        this.pointer = 0;
        this.relaxTime = false;
        this.range = range;
        this.setNumber = setNumber;
        this.hotTimes = hotTimes;
        this.relaxTimes = relaxTimes;
        this.setBegins = setBegins;
    }


    private void update_pointer() {
        if (!relaxTime) {
            if (time >= hotTimes[pointer]) {
                time = 0;
                relaxTime = true;
            }
        } else {
            if (time >= relaxTimes[pointer]) {
                time = 0;
                relaxTime = false;
                ++pointer;
                if (pointer >= setNumber) {
                    pointer = 0;
                }
            }
        }
        ++time;
    }

    private int next() {
        update_pointer();
        int value;
        if (relaxTime) {
            value = data.get(relaxDist.next());
        } else {
            int index = setBegins[pointer] + hotDists[pointer].next();
            if (index >= range) {
                index -= range;
            }
            value = data.get(index);
        }

        return value;
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

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }
}
