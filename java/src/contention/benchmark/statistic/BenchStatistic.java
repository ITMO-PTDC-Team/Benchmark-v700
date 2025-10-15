package contention.benchmark.statistic;

import static contention.benchmark.tools.StringFormat.*;

public class BenchStatistic {
    public double throughput;
    //    public double averageTraversalLength;
    public long effectiveUpdates;
    public long finalSize;
    /**
     * #nodes (inc. deleted)
     * if the data structure is not instance of MaintenanceAlg, the numNodes = -1
     */
    public long numNodes = -1;
    public ThreadStatistic[] threadStatistics;
    public ThreadStatistic commonStatistic;
    public double elapsedTime;
    public long prefillSize;

    public String toString() {
        return toString(1);
    }

    public String toString(int indents) {
        StringBuilder stringBuilder = new StringBuilder()
                .append(indentedTitleWithData("Average traversal length",
                        (double) commonStatistic.nodesTraversed / (double) commonStatistic.getCount, indents))
                .append(indentedTitleWithData("Struct Modifications", commonStatistic.structMods, indents))
                .append(indentedTitleWithData("Throughput (ops/s)", throughput, indents))
                .append(indentedTitleWithData("Elapsed time (s)", elapsedTime, indents))
                .append(indentedTitleWithDataPercent("Operations", commonStatistic.total, 1, indents))
                .append(indentedTitleWithDataPercent("effective updates",
                        commonStatistic.numAdd + commonStatistic.numRemove
                                + commonStatistic.numAddAll + commonStatistic.numRemoveAll,
                        ((double) (commonStatistic.numAdd + commonStatistic.numRemove
                                + commonStatistic.numAddAll + commonStatistic.numRemoveAll))
                                / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("|--add successful",
                        commonStatistic.numAdd,
                        ((double) commonStatistic.numAdd / (double) commonStatistic.total),
                        indents + 1))
                .append(indentedTitleWithDataPercent("|--remove succ.",
                        commonStatistic.numRemove,
                        (double) commonStatistic.numRemove / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("|--rangeQuery succ.",
                        commonStatistic.numRangeQuery,
                        (double) commonStatistic.numRangeQuery / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("|--addAll succ.",
                        commonStatistic.numAddAll,
                        (double) commonStatistic.numAddAll / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("|--removeAll succ.",
                        commonStatistic.numRemoveAll,
                        (double) commonStatistic.numRemoveAll / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("size successful",
                        commonStatistic.numSize,
                        (double) commonStatistic.numSize / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("contains succ.",
                        commonStatistic.numContains,
                        (double) commonStatistic.numContains / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithDataPercent("unsuccessful ops",
                        commonStatistic.numContains,
                        (double) commonStatistic.numContains / (double) commonStatistic.total,
                        indents + 1))
                .append(indentedTitleWithData("Final size", finalSize, indents));

        if (numNodes != -1) {
            stringBuilder
                    .append(indentedTitleWithData("#nodes (inc. deleted)", numNodes, indents));
        }

        return stringBuilder.toString();
    }

}
