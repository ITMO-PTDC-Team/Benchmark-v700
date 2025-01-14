package contention.benchmark.workload.data.map.builders;

import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.impls.KeysArrayDataMap;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class KeyArrayDataMapBuilder extends DataMapBuilder {
    transient public int range;
    transient private int[] data;
    transient private String filename;

    @Override
    public DataMapBuilder init(int range) {
        this.range = range;
        return this;
    }

    @Override
    public DataMap build() {
        readFile();

        return new KeysArrayDataMap(-range, data);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "KeysArrayDataMap", indents));
    }

    public KeyArrayDataMapBuilder setFileName(String filename) {
        this.filename = filename;
        return this;
    }

    public void readFile() {
        try {
            FileInputStream fin = new FileInputStream(filename);
            BufferedInputStream bin = new BufferedInputStream(fin);
            DataInputStream stream = new DataInputStream(bin);
            int fileSize = stream.available() / 4;
            this.data = new int[fileSize];
            for (int i = 0; i < fileSize - 1; ++i) {
                data[i] = stream.readInt();
            }
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }
}
