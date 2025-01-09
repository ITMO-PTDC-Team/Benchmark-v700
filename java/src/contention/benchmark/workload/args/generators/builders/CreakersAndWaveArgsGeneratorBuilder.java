package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.impls.CreakersAndWaveArgsGenerator;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;
import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.abstractions.MutableDistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.distributions.builders.ZipfianDistributionBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;

import java.util.concurrent.atomic.AtomicInteger;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class CreakersAndWaveArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    public double creakersSize = 0;
    public double creakersRatio = 0;
    public double waveSize = 0;
    public DistributionBuilder creakersDistBuilder = new UniformDistributionBuilder();
    public MutableDistributionBuilder waveDistBuilder = new ZipfianDistributionBuilder();

    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();


    transient private int creakersLength;
    transient private int creakersBegin;
    transient private AtomicInteger waveBegin;
    transient private AtomicInteger waveEnd;


    public CreakersAndWaveArgsGeneratorBuilder setCreakersSize(double creakersSize) {
        this.creakersSize = creakersSize;
        return this;
    }

    public CreakersAndWaveArgsGeneratorBuilder setCreakersRatio(double creakersRatio) {
        this.creakersRatio = creakersRatio;
        return this;
    }

    public CreakersAndWaveArgsGeneratorBuilder setWaveSize(double waveSize) {
        this.waveSize = waveSize;
        return this;
    }

    public CreakersAndWaveArgsGeneratorBuilder setCreakersDistBuilder(DistributionBuilder creakersDistBuilder) {
        this.creakersDistBuilder = creakersDistBuilder;
        return this;
    }

    public CreakersAndWaveArgsGeneratorBuilder setWaveDistBuilder(MutableDistributionBuilder waveDistBuilder) {
        this.waveDistBuilder = waveDistBuilder;
        return this;
    }

    public CreakersAndWaveArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    public CreakersAndWavePrefillArgsGeneratorBuilder generateDefaultPrefillArgsGeneratorBuilder() {
        return new CreakersAndWavePrefillArgsGeneratorBuilder(this);
    }

    @Override
    public CreakersAndWaveArgsGeneratorBuilder init(int range) {
        creakersLength = (int) (range * creakersSize);
        creakersBegin = range - creakersLength;
        waveEnd = new AtomicInteger(creakersBegin);
        waveBegin = new AtomicInteger(waveEnd.get() - (int) (range * waveSize));
        // dataMapBuilder.init(range);
        return this;
    }

    @Override
    public ArgsGenerator build() {
        return new CreakersAndWaveArgsGenerator(creakersRatio, creakersBegin, waveBegin, waveEnd,
                creakersDistBuilder.build(creakersLength), waveDistBuilder.build(), dataMapBuilder.build()
        );
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "CREAKERS_AND_WAVE", indents))
                .append(indentedTitleWithData("Creakers Ratio", creakersRatio, indents))
                .append(indentedTitleWithData("Creakers Size", creakersSize, indents))
                .append(indentedTitleWithData("Wave Size", waveSize, indents))
                .append(indentedTitle("Creakers Distribution", indents))
                .append(creakersDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Wave Distribution", indents))
                .append(waveDistBuilder.toStringBuilder(indents + 1))
                .append(indentedTitle("Data Map", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }
}
