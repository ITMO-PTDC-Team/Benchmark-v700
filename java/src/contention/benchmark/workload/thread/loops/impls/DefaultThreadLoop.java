package contention.benchmark.workload.thread.loops.impls;

import contention.abstractions.DataStructure;
import contention.benchmark.tools.Pair;
import contention.benchmark.workload.thread.loops.abstractions.ThreadLoop;
import contention.benchmark.workload.thread.loops.parameters.RatioThreadLoopParameters;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;
import contention.benchmark.workload.stop.condition.StopCondition;

import java.lang.reflect.Method;
import java.util.Random;
import java.util.Vector;

public class DefaultThreadLoop extends ThreadLoop {
    private final ArgsGenerator argsGenerator;
    protected Random rand = new Random();

    /**
     * The distribution of methods as an array of percentiles
     * <p>
     * 0%        cdf[0]        cdf[2]                     100%
     * |--writeAll--|--writeSome--|--readAll--|--readSome--|
     * |-----------write----------|--readAll--|--readSome--| cdf[1]
     */
    private final double[] cdf = new double[5];

    public DefaultThreadLoop(int threadId, DataStructure<Integer> dataStructure,
                             Method[] methods, StopCondition stopCondition,
                             RatioThreadLoopParameters parameters, ArgsGenerator argsGenerator) {
        super(threadId, dataStructure, methods, stopCondition);
        /* initialize the method boundaries */
        this.argsGenerator = argsGenerator;
        cdf[0] = parameters.writeAllsRatio;
        cdf[1] = cdf[0] + parameters.insertRatio;
        cdf[2] = cdf[1] + parameters.removeRatio;
        cdf[3] = cdf[2] + parameters.rangeQueryRatio;
        cdf[4] = cdf[3] + parameters.snapshotsRatio;
    }

    @Override
    public void step() {
        double coin = rand.nextDouble();
        if (coin < cdf[0]) { // 1. should we run a writeAll operation?
            // todo: something very strange

            int key = argsGenerator.nextGet();

            // init a collection
            Vector<Integer> vec = new Vector<Integer>(key);
            vec.add(key / 2); // accepts duplicate

            executeRemoveAll(vec);
        } else if (coin < cdf[1]) { // 2. should we run an insert
            int key = argsGenerator.nextInsert();
            executeInsert(key);
        } else if (coin < cdf[2]) { // 3. should we run a remove
            int key = argsGenerator.nextRemove();
            executeRemove(key);
        } else if (coin < cdf[3]) {  // 4. should we run a range query
            Pair<Integer, Integer> keys = argsGenerator.nextRange();
            executeRangeQuery(keys);
        } else if (coin < cdf[4]) { // 5. should we run a readAll operation?
            executeSize();
        } else { //if (coin < cdf[3]) { // 5. then we should run a readSome operation
            int key = argsGenerator.nextGet();
            executeGet(key);
        }
    }

}
