package contention.benchmark.workload.thread.loops.parameters;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class RatioThreadLoopParameters {
    public double
            insertRatio = 0.2,
            removeRatio = 0.2,
            rangeQueryRatio = 0,
            writeAllsRatio = 0,
            snapshotsRatio = 0;


    public RatioThreadLoopParameters() {}

    public RatioThreadLoopParameters(double insertRatio, double removeRatio) {
        this.insertRatio = insertRatio;
        this.removeRatio = removeRatio;
    }

    public RatioThreadLoopParameters setInsertRatio(double insertRatio) {
        this.insertRatio = insertRatio;
        return this;
    }

    public RatioThreadLoopParameters setRemoveRatio(double removeRatio) {
        this.removeRatio = removeRatio;
        return this;
    }

    public RatioThreadLoopParameters setRangeQueryRatio(double removeRatio) {
        this.rangeQueryRatio = removeRatio;
        return this;
    }

    public RatioThreadLoopParameters setWriteRatio(double writeRatio) {
        this.insertRatio = writeRatio / 2;
        this.removeRatio = writeRatio / 2;
        return this;
    }

    public RatioThreadLoopParameters setWriteAllsRatio(double writeAllsRatio) {
        this.writeAllsRatio = writeAllsRatio;
        return this;
    }

    public RatioThreadLoopParameters setSnapshotsRatio(double snapshotsRatio) {
        this.snapshotsRatio = snapshotsRatio;
        return this;
    }

    public StringBuilder toStringBuilder() {
        return toStringBuilder(1);
    }

    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Insert ratio", insertRatio, indents))
                .append(indentedTitleWithData("Remove ratio", removeRatio, indents))
                .append(indentedTitleWithData("Range query ratio", rangeQueryRatio, indents))
                .append(indentedTitleWithData("WriteAll ratio", writeAllsRatio, indents))
                .append(indentedTitleWithData("Snapshot ratio", snapshotsRatio, indents));
    }
}

