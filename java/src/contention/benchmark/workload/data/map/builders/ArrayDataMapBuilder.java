package contention.benchmark.workload.data.map.builders;

import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.impls.ArrayDataMap;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class ArrayDataMapBuilder extends DataMapBuilder {
    List<Integer> dataList;

    public ArrayDataMapBuilder setDataList(int range) {
        dataList = IntStream.range(0, range).boxed().collect(Collectors.toList());
        Collections.shuffle(dataList);
        return this;
    }

    @Override
    public ArrayDataMapBuilder init(int range) {
        return setDataList(range);
    }

    @Override
    public DataMap build() {
        return new ArrayDataMap(dataList);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "ArrayDataMap", indents));
    }
}
