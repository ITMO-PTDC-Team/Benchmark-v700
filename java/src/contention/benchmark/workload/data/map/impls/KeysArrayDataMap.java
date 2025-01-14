package contention.benchmark.workload.data.map.impls;

import java.io.DataInputStream;

public class KeysArrayDataMap extends ArrayDataMap{
    public KeysArrayDataMap(int n, int[] stream) {
        super(-n);
        try {
            for (int i = 0; i < -n; i++) {
                data[i] = stream[i];
            }
        } catch (Exception e) {
            System.out.println(e.getMessage());
            return;
        }
    }
}
