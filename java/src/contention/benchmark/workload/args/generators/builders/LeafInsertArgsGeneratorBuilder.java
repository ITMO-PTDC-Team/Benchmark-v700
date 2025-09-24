package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.LeafInsertArgsGenerator;
import contention.benchmark.workload.data.map.impls.LeafInsertDataMap;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class LeafInsertArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    transient public int range;
    transient private LeafInsertDataMap data;


    @Override
    public LeafInsertArgsGeneratorBuilder init(int range) {
        this.range = range;
        data = new LeafInsertDataMap(range);

        return this;
    }

    @Override
    public LeafInsertArgsGenerator build() {
        return new LeafInsertArgsGenerator(data, range);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "LEAF_INSERT", indents));
    }
}
