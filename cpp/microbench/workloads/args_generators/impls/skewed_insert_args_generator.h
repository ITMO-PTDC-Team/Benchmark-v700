//
// Created by Ravil Galiev on 08.08.2023.
//
#pragma once

#include "workloads/args_generators/args_generator.h"

#include "globals_extern.h"
#include "errors.h"
#include "workloads/data_maps/data_map.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "workloads/distributions/distribution.h"

template <typename K>
class SkewedInsertArgsGenerator : public ArgsGenerator<K> {
    //    PAD;
    size_t skewed_length_;
    size_t inserted_number_;
    Distribution* distribution_;
    // PAD;
    DataMap<K>* data_map_;
    // PAD;

public:
    SkewedInsertArgsGenerator(size_t skewed_length, Distribution* distribution,
                              DataMap<K>* data_map)
        : inserted_number_(0),
          skewed_length_(skewed_length),
          distribution_(distribution),
          data_map_(data_map) {
    }

    K next_get() override{setbench_error("Unsupported operation -- nextGet")}

    K next_insert() override {
        K value;
        if (inserted_number_ < skewed_length_) {
            value = data_map_->get(inserted_number_++);
        } else {
            value = data_map_->get(skewed_length_ + distribution_->next());
        }
        return value;
    }

    K next_remove() override{setbench_error("Unsupported operation -- nextGet")}

    std::pair<K, K> next_range() override {
        setbench_error("Unsupported operation -- nextGet")
    }

    ~SkewedInsertArgsGenerator() override {
        delete distribution_;
        delete data_map_;
    };
};

#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"

// template<typename K>
class SkewedInsertArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t range_;

    DistributionBuilder* dist_builder_ = new UniformDistributionBuilder();

    DataMapBuilder* data_map_builder_ = new ArrayDataMapBuilder();

    double skewed_size_ = 0;

    size_t skewed_length_;

public:
    SkewedInsertArgsGeneratorBuilder* set_skewed_size(double skewed_size) {
        skewed_size_ = skewed_size;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* set_distribution_builder(DistributionBuilder* dist_builder) {
        dist_builder_ = dist_builder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* set_data_map_builder(DataMapBuilder* data_map_builder) {
        data_map_builder_ = data_map_builder;
        return this;
    }

    SkewedInsertArgsGeneratorBuilder* init(size_t range) override {
        range_ = range;
        //        dataMapBuilder->init(range);
        skewed_length_ = (size_t)(range * skewed_size_);
        return this;
    }

    SkewedInsertArgsGenerator<K>* build(Random64& rng) override {
        return new SkewedInsertArgsGenerator<K>(skewed_length_,
                                                dist_builder_->build(rng, range_ - skewed_length_),
                                                data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "SkewedInsertArgsGeneratorBuilder";
        j["distributionBuilder"] = *dist_builder_;
        j["skewedSize"] = skewed_size_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        dist_builder_ = get_distribution_from_json(j["distributionBuilder"]);
        skewed_size_ = j["skewedSize"];
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "SKEWED_INSERT", indents) +
               indented_title_with_data("Skewed size", skewed_size_, indents) +
               indented_title("Distribution", indents) + dist_builder_->to_string(indents + 1) +
               indented_title("Data Map", indents) + data_map_builder_->to_string(indents + 1);
    }

    ~SkewedInsertArgsGeneratorBuilder() override {
        delete dist_builder_;
        //        delete dataMapBuilder; //TODO may delete twice
    };
};
