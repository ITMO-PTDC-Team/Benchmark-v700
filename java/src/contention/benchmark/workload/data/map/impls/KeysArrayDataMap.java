package contention.benchmark.workload.data.map.impls;

import java.io.DataInputStream;

public class KeysArrayDataMap extends ArrayDataMap{
    public KeysArrayDataMap(int n, DataInputStream stream) {
        super(-n);
        System.out.println(n + "AAAAAAAAAAAAAAAAAA");
        //System.out.println(stream.toString());
        try {
            for (int i = 0; i < -n; i++) {
                System.out.println("BBBBBBBBBBBBBBBB");
                data[i] = stream.readInt();
                System.out.println(data[i]);
            }
        } catch (Exception e) {
            System.out.println(e.getMessage());
            return;
        }
    }
}
