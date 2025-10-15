package contention.benchmark.data.sctucrure;

import contention.abstractions.CompositionalSortedSet;
import contention.abstractions.DataStructure;

import java.util.Collection;

public class SortedSetDataStructure<K> implements DataStructure<K> {
    private final CompositionalSortedSet<K> dataStructure;

    public SortedSetDataStructure(CompositionalSortedSet<K> dataStructure) {
        this.dataStructure = dataStructure;
    }

    @Override
    public K insert(K key) {
        return dataStructure.add(key)? null : key;
    }

    @Override
    public K remove(K key) {
        return dataStructure.remove(key) ? key : null;
    }

    @Override
    public K get(K key) {
        return dataStructure.contains(key) ? key : null;
    }

    @Override
    public int rangeQuery(K lo, K hi) {
        // TODO: Make a proper implementation
        return 0;
    }

    @Override
    public boolean removeAll(Collection<K> c) {
        return dataStructure.removeAll(c);
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
