//
// Created by Ravil Galiev on 08.08.2023.
//

#ifndef SETBENCH_TEMPORARY_SKEWED_ARGS_GENERATOR_H
#define SETBENCH_TEMPORARY_SKEWED_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

#include "globals_extern.h"

/**
    n — { xi — yi — ti — rti } // либо   n — rt — { xi — yi — ti }
        n — количество элементов
        xi — процент элементов i-ого множества
        yi — вероятность чтения элемента из i-ого множества
          // 100% - yi — чтение остальных элементов
        ti — время / количество итераций работы в режиме горячего вызова i-ого множества
        rti / rt (relax time) — время / количество итераций работы в обычном режиме (равномерное распределение на все элементы)
          // rt — если relax time всегда одинаковый
          // rti — relax time после горячей работы с i-ым множеством
 */
template<typename K>
class TemporarySkewedArgsGenerator : public ArgsGenerator<K> {
    size_t time;
    size_t pointer;
    bool isRelaxTime;

    Distribution **hotDists;
    Distribution *relaxDist;
    DataMap<K> *dataMap;
    PAD;
    long long *hotTimes;
    PAD;
    long long *relaxTimes;
    PAD;
    size_t *setBegins;
    PAD;
    size_t setNumber;
    size_t range;

    void update_pointer() {
        if (!isRelaxTime) {
            if (time >= hotTimes[pointer]) {
                time = 0;
                isRelaxTime = true;
            }
        } else {
            if (time >= relaxTimes[pointer]) {
                time = 0;
                isRelaxTime = false;
                ++pointer;
                if (pointer >= setNumber) {
                    pointer = 0;
                }
            }
        }
        ++time;
    }

    K next() {
        update_pointer();
        K value;

        if (isRelaxTime) {
            value = dataMap->get(relaxDist->next());
        } else {
            size_t index = setBegins[pointer] + hotDists[pointer]->next();
            if (index >= range) {
                index -= range;
            }

            value = dataMap->get(index);
        }

        return value;
    }

public:
    TemporarySkewedArgsGenerator(size_t setNumber, size_t range,
                                 long long *hotTimes, long long *relaxTimes, size_t *setBegins,
                                 Distribution **hotDists, Distribution *relaxDist, DataMap<K> *dataMap)
            : hotDists(hotDists), relaxDist(relaxDist), dataMap(dataMap), hotTimes(hotTimes), relaxTimes(relaxTimes),
              setBegins(setBegins), setNumber(setNumber), range(range), time(0), pointer(0), isRelaxTime(false) {}

    K nextGet() override {
        return next();
    }

    K nextInsert() override {
        return next();
    }

    K nextRemove() override {
        return next();
    }

    std::pair<K, K> nextRange() override {
        --time;
        K left = next();
        K right = next();

        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~TemporarySkewedArgsGenerator() override {
        delete[] hotDists;
        delete relaxDist;
//        delete dataMap; //TODO may deleted twice
    };

};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"

class TemporarySkewedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range;
    size_t setNumber = 0;
    SkewedUniformDistributionBuilder **hotDistBuilders;
    DistributionBuilder *relaxDistBuilder = new UniformDistributionBuilder();
    PAD;
    long long *hotTimes;
    PAD;
    long long *relaxTimes;
    PAD;
    long long defaultHotTime = -1;
    long long defaultRelaxTime = -1;

    /**
     * manual setting of the begins of sets
     */
    bool manualSettingSetBegins = false;
    double *setBegins;
    PAD;
    size_t *setBeginIndexes;
    PAD;

    DataMapBuilder *dataMapBuilder = new ArrayDataMapBuilder();

public:
    TemporarySkewedArgsGeneratorBuilder *enableManualSettingSetBegins() {
        manualSettingSetBegins = true;
        setBegins = new double[setNumber];
        std::fill(setBegins, setBegins + setNumber, 0);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *disableManualSettingSetBegins() {
        manualSettingSetBegins = false;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setSetNumber(const size_t _setNumber) {
        setNumber = _setNumber;
        hotDistBuilders = new SkewedUniformDistributionBuilder *[_setNumber];
        hotTimes = new long long[setNumber];
        relaxTimes = new long long[setNumber];

        if (manualSettingSetBegins) {
            setBegins = new double[setNumber];
            std::fill(setBegins, setBegins + setNumber, 0);
        }

        /**
         * if hotTimes[point] == -1, we will use hotTime
         * relaxTime analogically
         */
        std::fill(hotTimes, hotTimes + setNumber, defaultHotTime);
        std::fill(relaxTimes, relaxTimes + setNumber, defaultRelaxTime);

        for (size_t i = 0; i < setNumber; ++i) {
            hotDistBuilders[i] = new SkewedUniformDistributionBuilder();
        }

        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setSetsDistBuilder(SkewedUniformDistributionBuilder **_setsDistBuilder) {
        hotDistBuilders = _setsDistBuilder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setSetDistBuilder(const size_t index,
                                                           SkewedUniformDistributionBuilder *_setDistBuilder) {
        assert(index < setNumber);
        hotDistBuilders[index] = _setDistBuilder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setRelaxDistBuilder(DistributionBuilder *_relaxDistBuilder) {
        relaxDistBuilder = _relaxDistBuilder;
        return this;
    }


    TemporarySkewedArgsGeneratorBuilder *
    setHotSizeAndRatio(const size_t index, const double _hotSize, const double _hotRatio) {
        assert(index < setNumber);
        hotDistBuilders[index]->setHotSize(_hotSize);
        hotDistBuilders[index]->setHotRatio(_hotRatio);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setHotSize(const size_t index, const double _hotSize) {
        assert(index < setNumber);
        hotDistBuilders[index]->setHotSize(_hotSize);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setHotRatio(const size_t index, const double _hotRatio) {
        assert(index < setNumber);
        hotDistBuilders[index]->setHotRatio(_hotRatio);
        return this;
    }


    TemporarySkewedArgsGeneratorBuilder *setHotTimes(long long *_hotTimes) {
        hotTimes = _hotTimes;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setRelaxTimes(long long *_relaxTimes) {
        relaxTimes = _relaxTimes;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setHotTime(const size_t index, const long long _hotTime) {
        assert(index < setNumber);

        hotTimes[index] = _hotTime;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setRelaxTime(const size_t index, const long long _relaxTime) {
        assert(index < setNumber);

        relaxTimes[index] = _relaxTime;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setDefaultHotTime(const long long _hotTime) {
        defaultHotTime = _hotTime;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setDefaultRelaxTime(const long long _relaxTime) {
        defaultRelaxTime = _relaxTime;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setSetBegins(double *_setBegins) {
        setBegins = _setBegins;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setSetBegin(const size_t index, const double _setBegin) {
        assert(index < setNumber);
        setBegins[index] = _setBegin;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *setDataMapBuilder(DataMapBuilder *_dataMapBuilder) {
        dataMapBuilder = _dataMapBuilder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder *init(size_t _range) override {
        range = _range;

        for (int i = 0; i < setNumber; ++i) {
            if (relaxTimes[i] == -1) {
                relaxTimes[i] = defaultRelaxTime;
            }
        }

        for (int i = 0; i < setNumber; ++i) {
            if (hotTimes[i] == -1) {
                hotTimes[i] = defaultHotTime;
            }
        }

        setBeginIndexes = new size_t[setNumber];


        if (manualSettingSetBegins) {
            for (size_t i = 0; i < setNumber; ++i) {
                setBeginIndexes[i] = (size_t) (range * setBegins[i]);
            }
        } else {
            size_t curIndex = 0;
            for (size_t i = 0; i < setNumber; ++i) {
                setBeginIndexes[i] = curIndex;
                curIndex += hotDistBuilders[i]->getHotLength(range);
            }
        }

//        dataMapBuilder->init(range);
        return this;
    }

    TemporarySkewedArgsGenerator<K> *build(Random64 &_rng) override {
        Distribution **hotDists = new Distribution *[setNumber];

        for (size_t i = 0; i < setNumber; ++i) {
            hotDists[i] = hotDistBuilders[i]->build(_rng, range);
        }

        return new TemporarySkewedArgsGenerator<K>(setNumber, range,
                                                   hotTimes, relaxTimes, setBeginIndexes,
                                                   hotDists, relaxDistBuilder->build(_rng, range),
                                                   dataMapBuilder->build());
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "TemporarySkewedArgsGeneratorBuilder";
        j["setNumber"] = setNumber;
        j["defaultHotTime"] = defaultHotTime;
        j["defaultRelaxTime"] = defaultRelaxTime;
        for (size_t i = 0; i < setNumber; ++i) {
            j["hotDistBuilders"].push_back(*hotDistBuilders[i]);
            j["hotTimes"].push_back(hotTimes[i]);
            j["relaxTimes"].push_back(relaxTimes[i]);
            if (manualSettingSetBegins) {
                j["setBegins"].push_back(setBegins[i]);
            }
        }
        j["relaxDistBuilder"] = *relaxDistBuilder;
        j["dataMapBuilder"] = *dataMapBuilder;
        j["manualSettingSetBegins"] = manualSettingSetBegins;
    }

    void fromJson(const nlohmann::json &j) override {
        manualSettingSetBegins = j["manualSettingSetBegins"];
        this->setDefaultHotTime(j["defaultHotTime"]);
        this->setDefaultRelaxTime(j["defaultRelaxTime"]);

        this->setSetNumber(j["setNumber"]);

        std::copy(std::begin(j["hotTimes"]), std::end(j["hotTimes"]), hotTimes);
        std::copy(std::begin(j["relaxTimes"]), std::end(j["relaxTimes"]), relaxTimes);

        if (manualSettingSetBegins) {
            std::copy(std::begin(j["setBegins"]), std::end(j["setBegins"]), setBegins);
        }

        relaxDistBuilder = getDistributionFromJson(j["relaxDistBuilder"]);
        dataMapBuilder = getDataMapFromJson(j["dataMapBuilder"]);

        size_t i = 0;
        for (const auto &j_i: j["hotDistBuilders"]) {
            hotDistBuilders[i] = dynamic_cast<SkewedUniformDistributionBuilder *>(
                    getDistributionFromJson(j_i)
            );
            ++i;
        }
    }

    std::string toString(size_t indents) override {
        std::string result =
                indented_title_with_str_data("Type", "TEMPORARY_SKEWED", indents)
                + indented_title_with_data("Set number", setNumber, indents)
                + indented_title_with_data("Default Hot Time", defaultHotTime, indents)
                + indented_title_with_data("Default Relax Time", defaultRelaxTime, indents)
                + indented_title_with_data("Manual Setting SetBegins", manualSettingSetBegins, indents)
                + indented_title("Hot Times", indents);

        for (size_t i = 0; i < setNumber; ++i) {
            result += indented_title_with_data("Hot Time " + std::to_string(i),
                                               hotTimes[i], indents + 1);
        }

        result += indented_title("Relax Times", indents);

        for (size_t i = 0; i < setNumber; ++i) {
            result += indented_title_with_data("Relax Time " + std::to_string(i),
                                               relaxTimes[i], indents + 1);
        }

        if (manualSettingSetBegins) {
            result += indented_title("Set Begins", indents);

            for (size_t i = 0; i < setNumber; ++i) {
                result += indented_title_with_data("Set Begin " + std::to_string(i),
                                                   setBegins[i], indents + 1);
            }
        }

        result += indented_title("Hot Distributions", indents);

        for (size_t i = 0; i < setNumber; ++i) {
            result += indented_title("Hot Distribution " + std::to_string(i), indents + 1)
                      + hotDistBuilders[i]->toString(indents + 2);
        }

        result +=
                indented_title("Relax Distribution", indents)
                + relaxDistBuilder->toString(indents + 1)
                + indented_title("Data Map", indents)
                + dataMapBuilder->toString(indents + 1);

        return result;
    }

    ~TemporarySkewedArgsGeneratorBuilder() override {
        delete[] hotTimes;
        delete[] relaxTimes;
        if (manualSettingSetBegins) {
            delete[] setBegins;
            delete[] setBeginIndexes;
        }
        delete[] hotDistBuilders;
        delete relaxDistBuilder;
//        delete dataMapBuilder;
    }


};

#endif //SETBENCH_TEMPORARY_SKEWED_ARGS_GENERATOR_H
