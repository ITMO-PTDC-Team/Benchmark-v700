package contention.abstractions;

import java.util.Collection;

public interface DataStructure<K> {
    K insert(K key);

    K remove(K key);

    K get(K key);

    int rangeQuery(K lo, K hi);

    boolean removeAll(Collection<K> c);

    int size();

    void clear();

//    String toString();

    Object getDataStructure();
}
