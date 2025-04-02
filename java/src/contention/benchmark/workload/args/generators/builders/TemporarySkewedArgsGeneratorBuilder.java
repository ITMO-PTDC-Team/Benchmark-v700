package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.TemporarySkewedArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.Distribution;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.builders.SkewedUniformDistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;

import java.util.Arrays;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class TemporarySkewedArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public int setNumber = 0;
    public int[] hotTimes;
    public int[] relaxTimes;
    public int defaultHotTime = -1;
    public int defaultRelaxTime = -1;
    public SkewedUniformDistributionBuilder[] hotDistBuilders;
    public DistributionBuilder relaxDistBuilder = new UniformDistributionBuilder();
    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();

    transient public int range;
    transient public int[] setBeginIndexes;

    public TemporarySkewedArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setSetNumber(final int setNumber) {
        this.setNumber = setNumber;
        hotDistBuilders = new SkewedUniformDistributionBuilder[setNumber];
        hotTimes = new int[setNumber];
        relaxTimes = new int[setNumber];

        /*
         * if hotTimes[i] == -1, we will use hotTime
         * relaxTime analogically
         */
        Arrays.fill(hotTimes, defaultHotTime);
        Arrays.fill(relaxTimes, defaultRelaxTime);
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setSetSize(final int i, final double setSize) {
        assert (i < setNumber);
        hotDistBuilders[i].setHotSize(setSize);
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setHotRatio(final int i, final double hotRatio) {
        assert (i < setNumber);
        hotDistBuilders[i].setHotRatio(hotRatio);
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setHotSizeAndRatio(final int i,
                                                                  final double setSize, final double hotRatio) {
        assert (i < setNumber);
        hotDistBuilders[i].setHotSize(setSize);
        hotDistBuilders[i].setHotRatio(hotRatio);
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setHotTimes(int[] hotTimes) {
        this.hotTimes = hotTimes;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setRelaxTimes(int[] relaxTimes) {
        this.relaxTimes = relaxTimes;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setHotDistBuilders(SkewedUniformDistributionBuilder[] hotDistBuilders) {
        this.hotDistBuilders = hotDistBuilders;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setRelaxDistBuilder(DistributionBuilder relaxDistBuilder) {
        this.relaxDistBuilder = relaxDistBuilder;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setHotTime(final int i, final int hotTime) {
        assert (i < setNumber);
        hotTimes[i] = hotTime;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setRelaxTime(final int i, final int relaxTime) {
        assert (i < setNumber);
        relaxTimes[i] = relaxTime;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setCommonHotTime(final int hotTime) {
        this.defaultHotTime = hotTime;
        return this;
    }

    public TemporarySkewedArgsGeneratorBuilder setCommonRelaxTime(final int relaxTime) {
        this.defaultRelaxTime = relaxTime;
        return this;
    }

    @Override
    public TemporarySkewedArgsGeneratorBuilder init(int range) {
        this.range = range;
//        dataMapBuilder.init(range);

        for (int i = 0; i < setNumber; ++i) {
            if (relaxTimes[i] == -1) {
                relaxTimes[i] = this.defaultRelaxTime;
            }
            if (hotTimes[i] == -1) {
                hotTimes[i] = this.defaultHotTime;
            }
        }

        setBeginIndexes = new int[setNumber];

        for (int i = 0, curIndex = 0; i < setNumber; ++i) {
            setBeginIndexes[i] = curIndex;
            curIndex += hotDistBuilders[i].getHotLength(range);
        }

        return this;
    }

    @Override
    public TemporarySkewedArgsGenerator build() {
        Distribution[] hotDists = new Distribution[setNumber];
        for (int i=0; i<setNumber; i++){
            hotDists[i] = hotDistBuilders[i].build(range);
        }
        return new TemporarySkewedArgsGenerator(setNumber, range, hotTimes, relaxTimes, setBeginIndexes,
                hotDists, relaxDistBuilder.build(range), dataMapBuilder.build()
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {

        StringBuilder result = new StringBuilder()
                .append(indentedTitleWithData("Type", "TEMPORARY_SKEWED", indents))
                .append(indentedTitleWithData("Set number", setNumber, indents))
                .append(indentedTitleWithData("Default Hot Time", defaultHotTime, indents))
                .append(indentedTitleWithData("Default Relax Time", defaultRelaxTime, indents))
//                .append(indentedTitleWithData("Manual Setting SetBegins", manualSettingSetBegins, indents))
                .append(indentedTitle("Hot Times", indents));

        for (int i = 0; i < setNumber; ++i) {
            result .append(indentedTitleWithData("Hot Time " + i, hotTimes[i], indents + 1));
        }

        result .append(indentedTitle("Relax Times", indents));

        for (int i = 0; i < setNumber; ++i) {
            result .append(indentedTitleWithData("Relax Time " + i,
                    relaxTimes[i], indents + 1));
        }
//
//        if (manualSettingSetBegins) {
//            result .append(indentedTitle("Set Begins", indents));
//
//            for (size_t i = 0; i < setNumber; ++i) {
//                result.append(indentedTitleWithData("Set Begin " + std::to_string(i),
//                        setBegins[i], indents + 1));
//            }
//        }

        result.append(indentedTitle("Hot Distributions", indents));

        for (int i = 0; i < setNumber; ++i) {
            result.append(indentedTitle("Hot Distribution " + i, indents + 1))
            .append(hotDistBuilders[i].toStringBuilder(indents + 2));
        }

        result.append(indentedTitle("Relax Distribution", indents))
        .append(relaxDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Data Map", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));

        return result;
    }

}
