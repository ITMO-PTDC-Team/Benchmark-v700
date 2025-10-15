package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.tools.Pair;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMap;

import java.util.Random;

public class CreakersAndWavePrefillArgsGenerator implements ArgsGenerator {
    private final DataMap data;
    private final int waveBegin;
    private final int prefillLength;
    private final Random random;

    public CreakersAndWavePrefillArgsGenerator(int  waveBegin, int prefillLength, DataMap data) {
        this.data = data;
        this.waveBegin = waveBegin;
        this.prefillLength = prefillLength;
        this.random = new Random();
    }

    @Override
    public int nextGet() {
        throw new UnsupportedOperationException("Get not supported");
    }

    @Override
    public int nextInsert() {
        return data.get(waveBegin + random.nextInt(prefillLength));
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
