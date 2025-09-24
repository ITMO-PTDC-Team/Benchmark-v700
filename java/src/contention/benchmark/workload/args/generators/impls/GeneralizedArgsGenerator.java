package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;

public class GeneralizedArgsGenerator implements ArgsGenerator {
    private final ArgsGenerator getGenerator;
    private final ArgsGenerator insertGenerator;
    private final ArgsGenerator removeGenerator;
    private final ArgsGenerator rangeGenerator;

    public GeneralizedArgsGenerator(ArgsGenerator getGenerator,
                                    ArgsGenerator insertGenerator,
                                    ArgsGenerator removeGenerator,
                                    ArgsGenerator rangeGenerator) {
        this.getGenerator = getGenerator;
        this.insertGenerator = insertGenerator;
        this.removeGenerator = removeGenerator;
        this.rangeGenerator = rangeGenerator;
    }

    @Override
    public int nextGet() {
        return getGenerator.nextGet();
    }

    @Override
    public int nextInsert() {
        return insertGenerator.nextInsert();
    }

    @Override
    public int nextRemove() {
        return removeGenerator.nextRemove();
    }

    @Override
    public int[] nextRange() {
        return rangeGenerator.nextRange();
    }
}