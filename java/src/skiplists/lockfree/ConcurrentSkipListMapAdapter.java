package skiplists.lockfree;

import contention.abstractions.CompositionalMap;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Collection;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListMap;

public class ConcurrentSkipListMapAdapter implements CompositionalMap<Integer, Integer> {

    private final ConcurrentSkipListMap<Integer, Integer> map;

    public ConcurrentSkipListMapAdapter() {
        map = new ConcurrentSkipListMap<>();
    }

    @Override
    public int size() {
        return 0;
    }

    @Override
    public boolean containsKey(Object key) {
        return map.containsKey(key);
    }

    @Override
    public boolean containsValue(Object value) {
        return map.containsValue(value);
    }

    @Override
    public Integer get(Object key) {
        return map.get(key);
    }

    @Nullable
    @Override
    public Integer put(Integer key, Integer value) {
        return map.put(key, value);
    }

    @Override
    public Integer remove(Object key) {
        return map.remove(key);
    }

    @Override
    public void putAll(@NotNull Map<? extends Integer, ? extends Integer> m) {
        map.putAll(m);
    }

    @Override
    public Integer putIfAbsent(Integer x, Integer y) {
        return map.putIfAbsent(x, y);
    }

    @Override
    public void clear() {
        map.clear();
    }

    @NotNull
    @Override
    public Set<Integer> keySet() {
        return map.keySet();
    }

    @NotNull
    @Override
    public Collection<Integer> values() {
        return map.values();
    }

    @NotNull
    @Override
    public Set<Entry<Integer, Integer>> entrySet() {
        return map.entrySet();
    }

    @Override
    public int rangeQuery(Integer key1, Integer key2) {
        return map.subMap(key1, key2).size();
    }
}
