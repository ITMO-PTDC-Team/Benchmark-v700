package contention.benchmark.workload.args.generators.builders;

import contention.benchmark.workload.args.generators.abstractions.ArgsGeneratorBuilder;
import contention.benchmark.workload.args.generators.impls.CreakersAndWavePrefillArgsGenerator;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;
import contention.benchmark.workload.args.generators.abstractions.ArgsGenerator;

import static contention.benchmark.tools.StringFormat.indentedTitle;
import static contention.benchmark.tools.StringFormat.indentedTitleWithData;

public class CreakersAndWavePrefillArgsGeneratorBuilder implements ArgsGeneratorBuilder {
    transient private int waveBegin;
    transient private int prefillLength;

    public double creakersSize = 0;
    public double waveSize = 0;

    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();

    public CreakersAndWavePrefillArgsGeneratorBuilder() {
    }

    public CreakersAndWavePrefillArgsGeneratorBuilder(CreakersAndWaveArgsGeneratorBuilder builder) {
        setParametersByBuilder(builder);
    }

    CreakersAndWavePrefillArgsGeneratorBuilder setParametersByBuilder(CreakersAndWaveArgsGeneratorBuilder builder) {
        creakersSize = builder.creakersSize;
        waveSize = builder.waveSize;
        dataMapBuilder = builder.dataMapBuilder;
        return this;
    }

    public CreakersAndWavePrefillArgsGeneratorBuilder setCreakersSize(double creakersSize) {
        this.creakersSize = creakersSize;
        return this;
    }

    public CreakersAndWavePrefillArgsGeneratorBuilder setWaveSize(double waveSize) {
        this.waveSize = waveSize;
        return this;
    }

    public CreakersAndWavePrefillArgsGeneratorBuilder setDataMapBuilder(DataMapBuilder dataMapBuilder) {
        this.dataMapBuilder = dataMapBuilder;
        return this;
    }

    @Override
    public ArgsGeneratorBuilder init(int range) {
        // dataMapBuilder.init(range);
        prefillLength = (int) (range * creakersSize) + (int) (range * waveSize);
        waveBegin = range - prefillLength;
        return this;
    }

    @Override
    public ArgsGenerator build() {
        return new CreakersAndWavePrefillArgsGenerator(waveBegin, prefillLength, dataMapBuilder.build());
    }

    @Override
    public StringBuilder toStringBuilder(int indents) {
        return new StringBuilder()
                .append(indentedTitleWithData("Type", "CREAKERS_AND_WAVE_PREFILL", indents))
                .append(indentedTitleWithData("Creakers Size", creakersSize, indents))
                .append(indentedTitleWithData("Wave Size", waveSize, indents))
                .append(indentedTitle("Data Map", indents))
                .append(dataMapBuilder.toStringBuilder(indents + 1));
    }
}
