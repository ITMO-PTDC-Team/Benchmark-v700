package contention.benchmark.workload.data.map.builders;

import contention.benchmark.exceptions.BinaryFileDataMapException;

import java.io.*;
import java.util.Collections;
import java.util.List;

import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class BinaryFileDataMapBuilder extends ArrayDataMapBuilder {
    private String filename = "";
    private Boolean shuffle = false;

    private BinaryFileDataMapBuilder readFile(int range, boolean shuffle) {
        try {
            FileInputStream fin = new FileInputStream(this.filename);
            BufferedInputStream bin = new BufferedInputStream(fin);
            DataInputStream stream = new DataInputStream(bin);
            int fileSize = stream.available() / 4;

            if (fileSize < range) {
                throw new BinaryFileDataMapException("File too small. \n" +
                        "The BinaryFileDataMap size is " + range + ". " +
                        "The file size is " + fileSize + ".\n" +
                        "The number of keys in the file must be no less than the range of keys. ");
            }

            List<Integer> list = new java.util.ArrayList<>();
            for (int i = 0; i < range; ++i) {
                list.add(stream.readInt());
            }
            if (shuffle) {
                Collections.shuffle(list);
            }
            data = list.stream().mapToInt(Integer::intValue).toArray();
        } catch (FileNotFoundException e) {
            throw new BinaryFileDataMapException("File \""+filename+"\" for BinaryFileDataMap not found. ", e);
        } catch (IOException e) {
            throw new BinaryFileDataMapException(e);
        }
        return this;
    }

    @Override
    public BinaryFileDataMapBuilder init(int range) {
        return readFile(range, shuffle);
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder(indentedTitleWithData("Type", "BinaryFileDataMap", indents))
                .append(indentedTitleWithData("File name", filename, indents))
                .append(indentedTitleWithData("Shuffled", shuffle, indents));
    }

    public BinaryFileDataMapBuilder setFilename(String filename) {
        this.filename = filename;
        return this;
    }

    public BinaryFileDataMapBuilder setShuffleFlag(Boolean shuffle) {
        this.shuffle = shuffle;
        return this;
    }

    public BinaryFileDataMapBuilder enableShuffleFlag() {
        this.shuffle = true;
        return this;
    }

    public BinaryFileDataMapBuilder disableShuffleFlag() {
        this.shuffle = false;
        return this;
    }


}
