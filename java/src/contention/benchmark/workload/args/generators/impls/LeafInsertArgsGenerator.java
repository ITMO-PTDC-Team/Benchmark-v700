package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.data.map.impls.LeafInsertDataMap;

import java.util.Random;

public class LeafInsertArgsGenerator implements ArgsGenerator {
    private final LeafInsertDataMap data;

    private final int range;
    private final Random random;
    private int curInsertLayers;
    private int curEraseLayers;
    private int insertIndex;
    private int eraseIndex;


    public LeafInsertArgsGenerator(LeafInsertDataMap data, int range) {
        this.data = data;
        this.range = range;
        this.random = new Random();
        this.curInsertLayers = 0;
        this.curEraseLayers = 0;
        this.insertIndex = 0;
        this.eraseIndex = 0;
    }

    @Override
    public int nextGet() {
//        int randLayer = random.nextInt(curInsertLayers);
//        int randIndex = random.nextInt(data.getLayerSize(randLayer));
//        return data.get(randLayer, randIndex);
        return random.nextInt(range);
    }

    @Override
    public int nextInsert() {
        int value = data.get(curInsertLayers, insertIndex++);

        if (insertIndex >= data.getLayerSize(curInsertLayers)) {
            insertIndex = 0;
            curInsertLayers++;

            if (curInsertLayers >= data.getLayersCount()) {
                curInsertLayers = 0;
            }
        }

        return value;
    }

    @Override
    public int nextRemove() {
        int value = data.get(curEraseLayers, eraseIndex++);

        if (eraseIndex > insertIndex && curEraseLayers == curInsertLayers) {
            eraseIndex = insertIndex;
        } else if (eraseIndex >= data.getLayerSize(curEraseLayers)) {
            eraseIndex = 0;
            curEraseLayers++;

            if (curEraseLayers >= data.getLayersCount()) {
                curEraseLayers = 0;
            }
        }

        return value;
    }

    @Override
    public int[] nextRange() {
        throw new UnsupportedOperationException("Range Query not supported");
    }

}
