package contention.benchmark.workload.data.map.builders;

import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.impls.ArrayDataMap;

import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class ArrayDataMapBuilder extends DataMapBuilder {
    transient int[] data;

    private ArrayDataMapBuilder generateDataList(int range) {
        List<Integer> list = new java.util.ArrayList<>(IntStream.range(0, range).boxed().toList());
        Collections.shuffle(list);
        data = list.stream().mapToInt(Integer::intValue).toArray();
        return this;
    }

    @Override
    public ArrayDataMapBuilder init(int range) {
        return generateDataList(range);
    }

    @Override
    public DataMap build() {
        return new ArrayDataMap(data);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "ArrayDataMap", indents));
    }
}
