package contention.benchmark.workload.args.generators.parameters;

import contention.benchmark.workload.distributions.abstractions.DistributionBuilder;
import contention.benchmark.workload.distributions.abstractions.MutableDistributionBuilder;
import contention.benchmark.workload.distributions.builders.UniformDistributionBuilder;
import contention.benchmark.workload.distributions.builders.ZipfianDistributionBuilder;
import contention.benchmark.workload.data.map.abstractions.DataMapBuilder;
import contention.benchmark.workload.data.map.builders.ArrayDataMapBuilder;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * старички + волна
 * n — g — gx — gk — cp — ca
 * n — количество элементов
 * g (grand) — процент старичков
 * // 100% - g — процент новичков
 * gy — вероятность их вызова
 * // 100% - gx — вероятность вызова новичков
 * gk — на сколько стары наши старички
 * | преподсчёт - перед началом теста делаем gk количество запросов к старичкам
 * cp (child) — распределение вызовов среди новичков
 * // по умолчанию Zipf 1
 * // при желании можно сделать cx — cy
 * ca (child add) — вероятность добавления нового элемента
 * // 100% - gx - ca — чтение новичка, ca добавление нового новичка
 * // при желании можно переработать на "100% - ca — чтение, ca — запись"
 * (то есть брать ca не от общей вероятности, а только при чтении"
 */
public class CreakersAndWaveParameters {
    public double creakersSize = 0;
    public double creakersProb = 0;
    public double waveSize = 0;
    public DistributionBuilder creakersDistBuilder = new UniformDistributionBuilder();
    public MutableDistributionBuilder waveDistBuilder = new ZipfianDistributionBuilder();

    public DataMapBuilder dataMapBuilder = new ArrayDataMapBuilder();


    transient public int creakersLength;
    transient public int defaultWaveLength;
    transient public int creakersBegin;
    transient public AtomicInteger waveBegin;
    transient public AtomicInteger waveEnd;
    transient public int prefillSize;


    public void init(int range) {
        creakersLength = (int) (range * creakersSize);
        creakersBegin = range - creakersLength;
        defaultWaveLength = (int) (range * waveSize);
        waveEnd = new AtomicInteger(creakersBegin);
        waveBegin = new AtomicInteger(waveEnd.get() - defaultWaveLength);
        prefillSize = creakersLength + defaultWaveLength;
//        dataMapBuilder.init(range);
    }

//    @Override
//    public boolean parseArg(ParseArgument args) {
//        switch (args.getCurrent()) {
//            case "-gs", "-cs" -> this.CREAKERS_SIZE = Double.parseDouble(args.getNext());
//            case "-gp", "-cp" -> this.CREAKERS_PROB = Double.parseDouble(args.getNext());
//            case "-ws" -> this.WAVE_SIZE = Double.parseDouble(args.getNext());
//            case "-g-age", "-c-age" -> this.CREAKERS_AGE = Integer.parseInt(args.getNext());
//            case "-g-dist", "-c-dist" -> this.creakersDistBuilder.parseDistribution(args);
//            case "-w-dist" -> this.waveDistBuilder.parseDistribution(args);
//            case "--size", "-i" -> {
//                args.getNext();
//                System.err.println("CreakersAndWave key generator does not accept prefill size argument. Ignoring...");
//            }
//            default -> {
//                return false;
//            }
//        }
//        return true;
//    }

//    @Override
    public StringBuilder toStringBuilder() {
        //TODO toStringBuilder
        StringBuilder params = new StringBuilder();
        params
                .append("\n")
                .append("  Key Generator:           \tCREAKERS_AND_WAVE")
                .append("\n")
                .append("  Creakers size:           \t")
                .append(this.creakersSize)
                .append("\n")
                .append("  Wave size:               \t")
                .append(this.waveSize)
                .append("\n")
                .append("  Creakers probability:    \t")
                .append(this.creakersProb)
                .append("\n")
//                .append("  Creakers age:            \t")
//                .append(this.creakersAge)
//                .append("\n")
                .append("  Creakers distribution:   \t")
//                .append(creakersDistBuilder.type)
//                .append(creakersDistBuilder.toStringBuilderParameters())
                .append("\n")
                .append("  Wave distribution:       \t");
//                .append(waveDistBuilder.type)
//                .append(waveDistBuilder.toStringBuilderParameters());

        return params;
    }
}
