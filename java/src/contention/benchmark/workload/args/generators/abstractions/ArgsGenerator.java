package contention.benchmark.workload.args.generators.abstractions;
import contention.benchmark.tools.Pair;

public interface ArgsGenerator {
    int nextGet();

    int nextInsert();

    int nextRemove();

    Pair<Integer, Integer> nextRange();
}
