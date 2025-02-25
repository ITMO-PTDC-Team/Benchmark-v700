package contention.benchmark.workload.data.map.builders;

import contention.benchmark.workload.data.map.abstractions.DataMap;
import contention.benchmark.workload.data.map.impls.ArrayDataMap;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class FileBasedDataMapBuilder extends ArrayDataMapBuilder {
    private String filename = "";

    private ArrayDataMapBuilder readFile(int range, boolean shuffle) {
        try {
            FileInputStream fin = new FileInputStream(this.filename);
            BufferedInputStream bin = new BufferedInputStream(fin);
            DataInputStream stream = new DataInputStream(bin);
            int fileSize = stream.available() / 4;
            List<Integer> list = new java.util.ArrayList<>();
            for (int i = 0; i < range || i < fileSize; ++i) {
                list.add(stream.readInt());
            }
            if (shuffle) {
                Collections.shuffle(list);
            }
            data = list.stream().mapToInt(Integer::intValue).toArray();
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
        return this;
    }

    @Override
    public ArrayDataMapBuilder init(int range) {
        return readFile(range, shuffle);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "FileBasedDataMap", indents));
    }

    public ArrayDataMapBuilder setFilename(String filename) {
        this.filename = filename;
        return this;
    }
}
