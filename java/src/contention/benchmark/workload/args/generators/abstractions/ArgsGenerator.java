package contention.benchmark.workload.args.generators.abstractions;

public interface ArgsGenerator {
    int nextGet();

    int nextInsert();

    int nextRemove();

    int[] nextRange();
}
