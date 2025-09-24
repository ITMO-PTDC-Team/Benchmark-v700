package contention.benchmark.workload.args.generators.impls;

import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;

public class NullArgsGenerator implements ArgsGenerator {
    public NullArgsGenerator() {
        // Конструктор по умолчанию
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
        throw new UnsupportedOperationException("Range Query not supported");
    }
}