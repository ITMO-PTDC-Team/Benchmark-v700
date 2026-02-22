#pragma once

#include "workloads/args_generators/impls/temporary_skewed_args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"

namespace microbench::workload {

class TemporarySkewedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range_;
    size_t set_number_ = 0;
    SkewedUniformDistributionBuilder** hot_dist_builders_;
    DistributionBuilder* relax_dist_builder_ = new UniformDistributionBuilder();
    PAD;
    int64_t* hot_times_;
    PAD;
    int64_t* relax_times_;
    PAD;
    int64_t default_hot_time_ = -1;
    int64_t default_relax_time_ = -1;

    /**
     * manual setting of the begins of sets
     */
    bool manual_setting_set_begins_ = false;
    double* set_begins_;
    PAD;
    size_t* set_begin_indexes_;
    PAD;

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

public:
    TemporarySkewedArgsGeneratorBuilder* enable_manual_setting_set_begins() {
        manual_setting_set_begins_ = true;
        set_begins_ = new double[set_number_];
        std::fill(set_begins_, set_begins_ + set_number_, 0);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* disable_manual_setting_set_begins() {
        manual_setting_set_begins_ = false;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_set_number(const size_t set_number) {
        set_number_ = set_number;
        hot_dist_builders_ = new SkewedUniformDistributionBuilder*[set_number];
        hot_times_ = new int64_t[set_number_];
        relax_times_ = new int64_t[set_number_];

        if (manual_setting_set_begins_) {
            set_begins_ = new double[set_number_];
            std::fill(set_begins_, set_begins_ + set_number_, 0);
        }

        /**
         * if hotTimes[point] == -1, we will use hotTime
         * relaxTime analogically
         */
        std::fill(hot_times_, hot_times_ + set_number_, default_hot_time_);
        std::fill(relax_times_, relax_times_ + set_number_, default_relax_time_);

        for (size_t i = 0; i < set_number_; ++i) {
            hot_dist_builders_[i] = new SkewedUniformDistributionBuilder();
        }

        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_sets_dist_builder(
        SkewedUniformDistributionBuilder** sets_dist_builder) {
        hot_dist_builders_ = sets_dist_builder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_set_dist_builder(
        const size_t index, SkewedUniformDistributionBuilder* set_dist_builder) {
        assert(index < set_number_);
        hot_dist_builders_[index] = set_dist_builder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_relax_dist_builder(
        DistributionBuilder* relax_dist_builder) {
        relax_dist_builder_ = relax_dist_builder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_hot_size_and_ratio(const size_t index,
                                                                const double hot_size,
                                                                const double hot_ratio) {
        assert(index < set_number_);
        hot_dist_builders_[index]->set_hot_size(hot_size);
        hot_dist_builders_[index]->set_hot_ratio(hot_ratio);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_hot_size(const size_t index, const double hot_size) {
        assert(index < set_number_);
        hot_dist_builders_[index]->set_hot_size(hot_size);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_hot_ratio(const size_t index, const double hot_ratio) {
        assert(index < set_number_);
        hot_dist_builders_[index]->set_hot_ratio(hot_ratio);
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_hot_times(int64_t* hot_times) {
        hot_times_ = hot_times;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_relax_times(int64_t* relax_times) {
        relax_times_ = relax_times;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_hot_time(const size_t index, const int64_t hot_time) {
        assert(index < set_number_);

        hot_times_[index] = hot_time;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_relax_time(const size_t index,
                                                        const int64_t relax_time) {
        assert(index < set_number_);

        relax_times_[index] = relax_time;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_default_hot_time(const int64_t hot_time) {
        default_hot_time_ = hot_time;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_default_relax_time(const int64_t relax_time) {
        default_relax_time_ = relax_time;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_set_begins(double* set_begins) {
        set_begins_ = set_begins;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_set_begin(const size_t index, const double set_begin) {
        assert(index < set_number_);
        set_begins_[index] = set_begin;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    TemporarySkewedArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;

        for (int i = 0; i < set_number_; ++i) {
            if (relax_times_[i] == -1) {
                relax_times_[i] = default_relax_time_;
            }
        }

        for (int i = 0; i < set_number_; ++i) {
            if (hot_times_[i] == -1) {
                hot_times_[i] = default_hot_time_;
            }
        }

        set_begin_indexes_ = new size_t[set_number_];

        if (manual_setting_set_begins_) {
            for (size_t i = 0; i < set_number_; ++i) {
                set_begin_indexes_[i] = (size_t)(range_ * set_begins_[i]);
            }
        } else {
            size_t cur_index = 0;
            for (size_t i = 0; i < set_number_; ++i) {
                set_begin_indexes_[i] = cur_index;
                cur_index += hot_dist_builders_[i]->get_hot_length(range_);
            }
        }

        //        dataMapBuilder->init(range);
        return this;
    }

    TemporarySkewedArgsGenerator<K>* build(Random64& rng) override {
        Distribution** hot_dists = new Distribution*[set_number_];

        for (size_t i = 0; i < set_number_; ++i) {
            hot_dists[i] = hot_dist_builders_[i]->build(rng, range_);
        }

        return new TemporarySkewedArgsGenerator<K>(
            set_number_, range_, hot_times_, relax_times_, set_begin_indexes_, hot_dists,
            relax_dist_builder_->build(rng, range_), data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "TemporarySkewedArgsGeneratorBuilder";
        j["setNumber"] = set_number_;
        j["defaultHotTime"] = default_hot_time_;
        j["defaultRelaxTime"] = default_relax_time_;
        for (size_t i = 0; i < set_number_; ++i) {
            j["hotDistBuilders"].push_back(*hot_dist_builders_[i]);
            j["hotTimes"].push_back(hot_times_[i]);
            j["relaxTimes"].push_back(relax_times_[i]);
            if (manual_setting_set_begins_) {
                j["setBegins"].push_back(set_begins_[i]);
            }
        }
        j["relaxDistBuilder"] = *relax_dist_builder_;
        j["dataMapBuilder"] = *data_map_builder_;
        j["manualSettingSetBegins"] = manual_setting_set_begins_;
    }

    void from_json(const nlohmann::json& j) override {
        manual_setting_set_begins_ = j["manualSettingSetBegins"];
        this->set_default_hot_time(j["defaultHotTime"]);
        this->set_default_relax_time(j["defaultRelaxTime"]);

        this->set_set_number(j["setNumber"]);

        std::copy(std::begin(j["hotTimes"]), std::end(j["hotTimes"]), hot_times_);
        std::copy(std::begin(j["relaxTimes"]), std::end(j["relaxTimes"]), relax_times_);

        if (manual_setting_set_begins_) {
            std::copy(std::begin(j["setBegins"]), std::end(j["setBegins"]), set_begins_);
        }

        relax_dist_builder_ = get_distribution_from_json(j["relaxDistBuilder"]);
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);

        size_t i = 0;
        for (const auto& j_i : j["hotDistBuilders"]) {
            hot_dist_builders_[i] =
                dynamic_cast<SkewedUniformDistributionBuilder*>(get_distribution_from_json(j_i));
            ++i;
        }
    }

    std::string to_string(size_t indents) override {
        std::string result =
            indented_title_with_str_data("Type", "TEMPORARY_SKEWED", indents) +
            indented_title_with_data("Set number", set_number_, indents) +
            indented_title_with_data("Default Hot Time", default_hot_time_, indents) +
            indented_title_with_data("Default Relax Time", default_relax_time_, indents) +
            indented_title_with_data("Manual Setting SetBegins", manual_setting_set_begins_,
                                     indents) +
            indented_title("Hot Times", indents);

        for (size_t i = 0; i < set_number_; ++i) {
            result += indented_title_with_data("Hot Time " + std::to_string(i), hot_times_[i],
                                               indents + 1);
        }

        result += indented_title("Relax Times", indents);

        for (size_t i = 0; i < set_number_; ++i) {
            result += indented_title_with_data("Relax Time " + std::to_string(i), relax_times_[i],
                                               indents + 1);
        }

        if (manual_setting_set_begins_) {
            result += indented_title("Set Begins", indents);

            for (size_t i = 0; i < set_number_; ++i) {
                result += indented_title_with_data("Set Begin " + std::to_string(i), set_begins_[i],
                                                   indents + 1);
            }
        }

        result += indented_title("Hot Distributions", indents);

        for (size_t i = 0; i < set_number_; ++i) {
            result += indented_title("Hot Distribution " + std::to_string(i), indents + 1) +
                      hot_dist_builders_[i]->to_string(indents + 2);
        }

        result += indented_title("Relax Distribution", indents) +
                  relax_dist_builder_->to_string(indents + 1) +
                  indented_title("Data Map", indents) + data_map_builder_->to_string(indents + 1);

        return result;
    }

    ~TemporarySkewedArgsGeneratorBuilder() override {
        delete[] hot_times_;
        delete[] relax_times_;
        if (manual_setting_set_begins_) {
            delete[] set_begins_;
            delete[] set_begin_indexes_;
        }
        delete[] hot_dist_builders_;
        delete relax_dist_builder_;
        //        delete dataMapBuilder;
    }
};

}  // namespace microbench::workload
