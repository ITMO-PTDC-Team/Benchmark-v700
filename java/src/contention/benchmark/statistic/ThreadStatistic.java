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
    public long insertNodesTraversed = 0;
    public long deleteNodesTraversed = 0;

    public long foundCnt;
    public long foundTreeTraversed;
    public long foundLogicalTraversed;
    public long notFoundCnt;
    public long notFoundTreeTraversed;
    public long notFoundLogicalTraversed;
    public long failedLockAcquire;

    public long foundCnt;
    public long foundTreeTraversed;
    public long foundLogicalTraversed;
    public long notFoundCnt;
    public long notFoundTreeTraversed;
    public long notFoundLogicalTraversed;
    public long failedLockAcquire;

    public long structMods;
    public long getCount;

    public void reset() {
        total = 0;
        numAdd = 0;
        numRemove = 0;
        numAddAll = 0;
        numRemoveAll = 0;
        numSize = 0;
        numContains = 0;
        failures = 0;
        aborts = 0;
        nodesTraversed = 0;
    	insertNodesTraversed = 0;
    	deleteNodesTraversed = 0;
        foundCnt = 0;
        foundTreeTraversed = 0;
        foundLogicalTraversed = 0;
        notFoundCnt = 0;
        notFoundTreeTraversed = 0;
        notFoundLogicalTraversed = 0;
        failedLockAcquire = 0;
        structMods = 0;
        getCount = 0;
    }

    public void add(ThreadStatistic stats) {
        total += stats.total;
        numAdd += stats.numAdd;
        numRemove += stats.numRemove;
        numAddAll += stats.numAddAll;
        numRemoveAll += stats.numRemoveAll;
        numSize += stats.numSize;
        numContains += stats.numContains;
        failures += stats.failures;
        aborts += stats.aborts;
        nodesTraversed += stats.nodesTraversed;
    	insertNodesTraversed += stats.insertNodesTraversed;
    	deleteNodesTraversed += stats.deleteNodesTraversed;
        foundCnt += stats.foundCnt;
        foundTreeTraversed += stats.foundTreeTraversed;
        foundLogicalTraversed += stats.foundLogicalTraversed;
        notFoundCnt += stats.notFoundCnt;
        notFoundTreeTraversed += stats.notFoundTreeTraversed;
        notFoundLogicalTraversed += stats.notFoundLogicalTraversed;
        failedLockAcquire += stats.failedLockAcquire;
        structMods += stats.structMods;
        getCount += stats.getCount;
    }

    public String toString() {
        return "";
    }

}
