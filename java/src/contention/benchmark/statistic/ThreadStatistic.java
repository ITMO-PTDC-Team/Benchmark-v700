package contention.benchmark.statistic;

public class ThreadStatistic {
    /**
     * The total number of operations for all threads
     */
    public long total = 0;

    /**
     * The total number of successful operations for all threads
     */
    public long numAdd = 0;
    public long numRemove = 0;
    public long numRangeQuery = 0;
    public long numAddAll = 0;
    public long numRemoveAll = 0;
    public long numSize = 0;
    public long numContains = 0;
    /**
     * The total number of failed operations for all threads
     */
    public long failures = 0;
    /**
     * The total number of aborts
     */
    public long aborts = 0;

    public long nodesTraversed;

    public long structMods;
    public long getCount;

    public void reset() {
        total = 0;
        numAdd = 0;
        numRemove = 0;
        numRangeQuery = 0;
        numAddAll = 0;
        numRemoveAll = 0;
        numSize = 0;
        numContains = 0;
        failures = 0;
        aborts = 0;
        nodesTraversed = 0;
        structMods = 0;
        getCount = 0;
    }

    public void add(ThreadStatistic stats) {
        total += stats.total;
        numAdd += stats.numAdd;
        numRangeQuery += stats.numRangeQuery;
        numRemove += stats.numRemove;
        numAddAll += stats.numAddAll;
        numRemoveAll += stats.numRemoveAll;
        numSize += stats.numSize;
        numContains += stats.numContains;
        failures += stats.failures;
        aborts += stats.aborts;
        nodesTraversed += stats.nodesTraversed;
        structMods += stats.structMods;
        getCount += stats.getCount;
    }

    public String toString() {
        return "";
    }

}
