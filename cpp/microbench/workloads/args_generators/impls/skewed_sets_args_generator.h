//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"
#include "workloads/data_maps/data_map.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "workloads/distributions/distribution.h"

namespace microbench::workload {

template <typename K>
class SkewedSetsArgsGenerator : public ArgsGenerator<K> {
    //    PAD;
    size_t range_;
    size_t write_set_begins_;
    Distribution* read_dist_;
    Distribution* write_dist_;
    DataMap<K>* data_map_;

    K next_write() {
        size_t index = write_set_begins_ + write_dist_->next();
        if (index >= range_) {
            index -= range_;
        }
        return data_map_->get(index);
    }

public:
    SkewedSetsArgsGenerator(size_t range, size_t write_set_begins, Distribution* read_dist,
                            Distribution* write_dist, DataMap<K>* data_map)
        : range_(range),
          write_set_begins_(write_set_begins),
          read_dist_(read_dist),
          write_dist_(write_dist),
          data_map_(data_map) {
    }

    K next_get() override {
        return data_map_->get(read_dist_->next());
    }

    K next_insert() override {
        return next_write();
    }

    K next_remove() override {
        return next_write();
    }

    std::pair<K, K> next_range() override {
        K left = next_get();
        K right = next_get();
        if (left > right) {
            std::swap(left, right);
        }
        return {left, right};
    }

    ~SkewedSetsArgsGenerator() override {
        delete read_dist_;
        delete write_dist_;
        delete data_map_;
    };
};

}  // namespace microbench::workload

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"

namespace microbench::workload {

class SkewedSetsArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range_;

    SkewedUniformDistributionBuilder* read_dist_builder_ = new SkewedUniformDistributionBuilder();
    SkewedUniformDistributionBuilder* write_dist_builder_ = new SkewedUniformDistributionBuilder();

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

    double intersection_ = 0;
    size_t write_set_begins_;

public:
    SkewedSetsArgsGeneratorBuilder* set_read_hot_size(double hot_size) {
        read_dist_builder_->set_hot_size(hot_size);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* set_read_hot_prob(double hot_prob) {
        read_dist_builder_->set_hot_ratio(hot_prob);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* set_write_hot_size(double hot_size) {
        write_dist_builder_->set_hot_size(hot_size);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* set_write_hot_prob(double hot_prob) {
        write_dist_builder_->set_hot_ratio(hot_prob);
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* set_intersection(double intersection) {
        intersection_ = intersection;
        return this;
    }

    SkewedSetsArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        //        dataMapBuilder->init(range);
        write_set_begins_ = read_dist_builder_->get_hot_length(range_) - range_ * intersection_;
        return this;
    }

    SkewedSetsArgsGenerator<K>* build(Random64& rng) override {
        return new SkewedSetsArgsGenerator<K>(
            range_, write_set_begins_, read_dist_builder_->build(rng, range_),
            write_dist_builder_->build(rng, range_), data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "SkewedSetsArgsGeneratorBuilder";
        j["readDistBuilder"] = *read_dist_builder_;
        j["writeDistBuilder"] = *write_dist_builder_;
        j["intersection"] = intersection_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        read_dist_builder_ = dynamic_cast<SkewedUniformDistributionBuilder*>(
            get_distribution_from_json(j["readDistBuilder"]));
        write_dist_builder_ = dynamic_cast<SkewedUniformDistributionBuilder*>(
            get_distribution_from_json(j["writeDistBuilder"]));
        intersection_ = j["intersection"];
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_SETS", indents) +
               indented_title_with_data("Intersection", intersection_, indents) +
               indented_title("Read Distribution", indents) +
               read_dist_builder_->to_string(indents + 1) +
               indented_title("Write Distribution", indents) +
               write_dist_builder_->to_string(indents + 1) + indented_title("Data Map", indents) +
               data_map_builder_->to_string(indents + 1);
    }

    ~SkewedSetsArgsGeneratorBuilder() override {
        delete read_dist_builder_;
        delete write_dist_builder_;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};

}  // namespace microbench::workload
