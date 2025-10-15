package contention.benchmark.data.sctucrure;

import contention.abstractions.CompositionalIntSet;
import contention.abstractions.DataStructure;

import java.util.Collection;

public class IntSetDataStructure implements DataStructure<Integer> {
    private final CompositionalIntSet dataStructure;

    public IntSetDataStructure(CompositionalIntSet dataStructure) {
        this.dataStructure = dataStructure;
    }

    @Override
    public Integer insert(Integer key) {
        return dataStructure.addInt(key) ? null : key;
    }

    @Override
    public Integer remove(Integer key) {
        return dataStructure.removeInt(key) ? key : null;
    }

    @Override
    public Integer get(Integer key) {
        return dataStructure.containsInt(key) ? key : null;
    }

    @Override
    public boolean removeAll(Collection<Integer> c) {
        return dataStructure.removeAll(c);
    }

    @Override
    public int rangeQuery(Integer lo, Integer hi) {
        // TODO: Make a proper implementation
        return 0;
    }

    @Override
    public int size() {
        return dataStructure.size();
    }

    @Override
    public void clear() {
        dataStructure.clear();
    }

    @Override
    public String toString() {
        return dataStructure.toString();
    }

    @Override
    public Object getDataStructure() {
        return dataStructure;
    }
}


