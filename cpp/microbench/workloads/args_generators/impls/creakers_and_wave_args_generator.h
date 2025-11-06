//
// Created by Ravil Galiev on 08.08.2023.
//

#ifndef SETBENCH_CREAKERS_AND_WAVE_ARGS_GENERATOR_H
#define SETBENCH_CREAKERS_AND_WAVE_ARGS_GENERATOR_H

#include <atomic>

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

#include "globals_extern.h"

/**
    старички + волна
    n — g — gx — gk — cp — ca
        n — количество элементов
        g (grand) — процент старичков
            // 100% - g — процент новичков
        gy — вероятность их вызова
            // 100% - gx — вероятность вызова новичков
        gk — на сколько стары наши старички
            | преподсчёт - перед началом теста делаем gk количество запросов к старичкам
        cp (child) — распределение вызовов среди новичков
            // по умолчанию Zipf 1
            // при желании можно сделать cx — cy
        ca (child add) — вероятность добавления нового элемента
            // 100% - gx - ca — чтение новичка, ca добавление нового новичка
            // при желании можно переработать на "100% - ca — чтение, ca — запись"
                (то есть брать ca не от общей вероятности, а только при чтении"
 */
template<typename K>
class CreakersAndWaveArgsGenerator : public ArgsGenerator<K> {
    PAD;
    Random64 &rng;
    PAD;
    double creakersRatio;
    size_t creakersBegin;
    Distribution *creakersDist;
    MutableDistribution *waveDist;
    DataMap<K> *dataMap;
    PAD;
    std::atomic<size_t> *waveBegin;
    PAD;
    std::atomic<size_t> *waveEnd;
    PAD;

    /**
     *
     * @return true - creakers, false - wave
     */
    bool next_coin() {
        double z; // Uniform random number (0 < z < 1)

        do {
            z = (rng.next() / (double) rng.max_value);
        } while ((z == 0) || (z == 1));

        return z < creakersRatio;
    }

    K getCreaker() {

        /**
         *                             creakersBegin
         * |________________________________|,,,,,,,,,,,,,,,,,,,,|
         *
         * |,,,,| --- creakers
         * |____| --- wave or unused data
         */

        return dataMap->get(creakersBegin + creakersDist->next());
    }

    K getWave() {
        /**
         * In waveDist the first indexes have a higher probability
         */
        size_t localWaveBegin = *waveBegin;
        size_t localWaveEnd = *waveEnd;
//            size_t localWaveLength = (localWaveEnd - localWaveBegin + creakersBegin) % creakersBegin;
        size_t localWaveLength;
        size_t index;
        if (localWaveEnd < localWaveBegin) {
            /**
             *       waveEnd                waveBegin      creakersBegin
             * |.......|________________________|.............|,,,,,,,,,,,,,,,,,,,,|
             *
             * |....| --- wave
             * |,,,,| --- creakers
             * |____| --- unused data
             */
            localWaveLength = (localWaveEnd + creakersBegin) - localWaveBegin;
            index = localWaveBegin + waveDist->next(localWaveLength);

            if (index >= creakersBegin) {
                index -= creakersBegin;
            }
        } else {
            localWaveLength = localWaveEnd - localWaveBegin;
            index = localWaveBegin + waveDist->next(localWaveLength);
        }

        return dataMap->get(index);
    };

    K waveShift(std::atomic<size_t> *waveEdge) {
        size_t localWaveEdge = *waveEdge;
        size_t newWaveEdge;
        if (localWaveEdge == 0) {
            newWaveEdge = creakersBegin - 1;
        } else {
            newWaveEdge = localWaveEdge - 1;
        }
        waveEdge->compare_exchange_weak(localWaveEdge, newWaveEdge);
        return dataMap->get(newWaveEdge);
    }
public:
    CreakersAndWaveArgsGenerator(Random64 &rng, double creakersRatio, size_t creakersBegin,
                                 std::atomic<size_t> *waveBegin,
                                 std::atomic<size_t> *waveEnd,
                                 Distribution *creakersDist,
                                 MutableDistribution *waveDist,
                                 DataMap<K> *dataMap) :
            rng(rng),
            creakersRatio(creakersRatio),
            creakersBegin(creakersBegin),
            waveBegin(waveBegin),
            waveEnd(waveEnd),
            creakersDist(creakersDist),
            waveDist(waveDist),
            dataMap(dataMap) {}

    K nextGet() override {
        K value;
        if (next_coin()) {
            value = getCreaker();
        } else {
            value = getWave();
        }
        return value;
    }


    K nextInsert() override {
        return waveShift(waveBegin);
    }

    K nextRemove() override {
        return waveShift(waveEnd);
    }

    std::pair<K, K> nextRange() override {
        K left;
        K right;
        if (next_coin()) {
            left = getCreaker();
            right = getCreaker();
        } else {
            left = getWave();
            right = getWave();
        }
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~CreakersAndWaveArgsGenerator() override {
        delete creakersDist;
        delete waveDist;
        delete dataMap;
    };
};

#include "errors.h"

template<typename K>
class CreakersAndWavePrefillArgsGenerator : public ArgsGenerator<K> {
    PAD;
    Random64 &rng;
    PAD;
    DataMap<K> *dataMap;
    size_t waveBegin;
    size_t prefillLength;
    PAD;

public:
    CreakersAndWavePrefillArgsGenerator(Random64 &rng, size_t waveBegin, size_t prefillLength, DataMap<K> *dataMap) :
            rng(rng), waveBegin(waveBegin), prefillLength(prefillLength), dataMap(dataMap) {}

    K nextGet() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    K nextInsert() override {
        /**
             *                       waveBegin           creakersBegin
             * |_________________________|....................|,,,,,,,,,,,,,,,,,,,,|
             *                           |<--          prefillLength            -->|
             * |....| --- wave
             * |,,,,| --- creakers
             * |____| --- unused data
         */
        return dataMap->get(waveBegin + rng.next(prefillLength));
    }

    K nextRemove() override {
        setbench_error("Unsupported operation -- nextRemove")
    }

    std::pair<K, K> nextRange() override {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~CreakersAndWavePrefillArgsGenerator() override {
        delete dataMap;
    };
};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"

//typedef long long K;

class CreakersAndWaveArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t creakersLength;
    size_t creakersBegin;
    size_t startWaveLength;
    PAD;
    std::atomic<size_t> *waveBegin;
    PAD;
    std::atomic<size_t> *waveEnd;
    PAD;


    double creakersSize = 0;
    double creakersRatio = 0;
    double waveSize = 0;

    DistributionBuilder *creakersDistBuilder = new UniformDistributionBuilder();
    MutableDistributionBuilder *waveDistBuilder = new ZipfianDistributionBuilder();

    DataMapBuilder *dataMapBuilder = new ArrayDataMapBuilder();

public:
    double getCreakersSize() const {
        return creakersSize;
    }

    double getWaveSize() const {
        return waveSize;
    }

    DataMapBuilder *getDataMapBuilder() const {
        return dataMapBuilder;
    }


    CreakersAndWaveArgsGeneratorBuilder *setCreakersSize(double _creakersSize) {
        creakersSize = _creakersSize;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *setCreakersRatio(double _creakersRatio) {
        creakersRatio = _creakersRatio;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *setWaveSize(double _waveSize) {
        waveSize = _waveSize;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *setCreakersDistBuilder(DistributionBuilder *_creakersDistBuilder) {
        creakersDistBuilder = _creakersDistBuilder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *setWaveDistBuilder(MutableDistributionBuilder *_waveDistBuilder) {
        waveDistBuilder = _waveDistBuilder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    CreakersAndWaveArgsGeneratorBuilder *init(size_t range) override {
//        dataMapBuilder->init(range);
        creakersLength = range * creakersSize;
        creakersBegin = range - creakersLength;
        startWaveLength = range * waveSize;
        waveEnd = new std::atomic<size_t>(creakersBegin);
        waveBegin = new std::atomic<size_t>(*waveEnd - startWaveLength);
        return this;
    }

    CreakersAndWaveArgsGenerator<K> *build(Random64 &_rng) override {
        return new CreakersAndWaveArgsGenerator<K>(_rng, creakersRatio, creakersBegin,
                                                   waveBegin,
                                                   waveEnd,
                                                   creakersDistBuilder->build(_rng, creakersLength),
                                                   waveDistBuilder->build(_rng),
                                                   dataMapBuilder->build());
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "CreakersAndWaveArgsGeneratorBuilder";
        j["creakersRatio"] = creakersRatio;
        j["creakersSize"] = creakersSize;
        j["waveSize"] = waveSize;
        j["creakersDistBuilder"] = *creakersDistBuilder;
        j["waveDistBuilder"] = *waveDistBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        creakersRatio = j["creakersRatio"];
        creakersSize = j["creakersSize"];
        waveSize = j["waveSize"];
        creakersDistBuilder = getDistributionFromJson(j["creakersDistBuilder"]);
        waveDistBuilder = getMutableDistributionFromJson(j["waveDistBuilder"]);
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);
    }

    std::string toString(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE", indents)
               + indented_title_with_data("Creakers Ratio", creakersRatio, indents)
               + indented_title_with_data("Creakers Size", creakersSize, indents)
               + indented_title_with_data("Wave Size", waveSize, indents)
               + indented_title("Creakers Distribution", indents)
               + creakersDistBuilder->toString(indents + 1)
               + indented_title("Wave Distribution", indents)
               + waveDistBuilder->toString(indents + 1)
               + indented_title("Data Map", indents)
               + dataMapBuilder->toString(indents + 1);
    }

    ~CreakersAndWaveArgsGeneratorBuilder() override {
        delete waveBegin;
        delete waveEnd;
        delete creakersDistBuilder;
        delete waveDistBuilder;
//        delete dataMapBuilder;
    };
};

class CreakersAndWavePrefillArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t waveBegin;
    size_t prefillLength;
    PAD;

    double creakersSize = 0;
    double waveSize = 0;

    DataMapBuilder *dataMapBuilder = new ArrayDataMapBuilder();

public:
    size_t getPrefillLength() const {
        return prefillLength;
    }

    size_t getPrefillLength(size_t range) const {
        return range * creakersSize + range * waveSize;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder() {}

    CreakersAndWavePrefillArgsGeneratorBuilder(CreakersAndWaveArgsGeneratorBuilder *builder) {
        setParametersByBuilder(builder);
    }

    CreakersAndWavePrefillArgsGeneratorBuilder *setParametersByBuilder(
                CreakersAndWaveArgsGeneratorBuilder *builder) {
        creakersSize = builder->getCreakersSize();
        waveSize = builder->getWaveSize();
        dataMapBuilder = builder->getDataMapBuilder();
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder *setCreakersSize(double _creakersSize) {
        creakersSize = _creakersSize;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder *setWaveSize(double _waveSize) {
        waveSize = _waveSize;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder *init(size_t range) override {
//        dataMapBuilder->init(range);
        prefillLength = range * creakersSize + range * waveSize;
        waveBegin = range - prefillLength;
        return this;
    }

    CreakersAndWavePrefillArgsGenerator<K> *build(Random64 &_rng) override {
        return new CreakersAndWavePrefillArgsGenerator<K>(_rng, waveBegin, prefillLength,
                                                          dataMapBuilder->build());
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "CreakersAndWavePrefillArgsGeneratorBuilder";
        j["creakersSize"] = creakersSize;
        j["waveSize"] = waveSize;
        j["dataMapBuilder"] = *dataMapBuilder;
    }

    void fromJson(const nlohmann::json &j) override {
        creakersSize = j["creakersSize"];
        waveSize = j["waveSize"];
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);
    }

    std::string toString(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE_PREFILL", indents)
               + indented_title_with_data("Creakers Size", creakersSize, indents)
               + indented_title_with_data("Wave Size", waveSize, indents)
               + indented_title("Data Map", indents)
               + dataMapBuilder->toString(indents + 1);
    }

    ~CreakersAndWavePrefillArgsGeneratorBuilder() override {
//        delete dataMapBuilder;
    };
};


#endif //SETBENCH_CREAKERS_AND_WAVE_ARGS_GENERATOR_H
