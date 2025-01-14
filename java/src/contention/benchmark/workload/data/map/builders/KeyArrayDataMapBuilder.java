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
    transient private DataInputStream stream;

    @Override
    public DataMapBuilder init(int range) {
        this.range = range;
        return this;
    }

    @Override
    public DataMap build() {
        return new KeysArrayDataMap(-range, stream);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "KeysArrayDataMap", indents));
    }

    public KeyArrayDataMapBuilder readFile(String filename) {
        try {
            FileInputStream fin = new FileInputStream(filename);
            BufferedInputStream bin = new BufferedInputStream(fin);
            this.stream = new DataInputStream(bin);
            System.out.println(stream.readInt());
        } catch (Exception e) {
            System.out.println(e.getMessage());
            //System.out.println("Cannot open file " + filename);
            //System.out.println(System.getProperty("user.dir"));
            //System.out.println(new File(".").getAbsolutePath());
        }
        return this;
    }
}
