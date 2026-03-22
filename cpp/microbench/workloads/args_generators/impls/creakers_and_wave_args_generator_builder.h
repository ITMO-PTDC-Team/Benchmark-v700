#pragma once

#include <memory>
#include "args_generators/impls/creakers_and_wave_args_generator.h"
#include "args_generators/impls/creakers_and_wave_prefill_args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/array_data_map_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"

namespace microbench::workload {

class CreakersAndWaveArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t creakers_length_;
    size_t creakers_begin_;
    size_t start_wave_length_;
    PAD;
    std::atomic<size_t> wave_begin_;
    PAD;
    std::atomic<size_t> wave_end_;
    PAD;

    double creakers_size_ = 0;
    double creakers_ratio_ = 0;
    double wave_size_ = 0;

    DistributionBuilderPtr creakers_dist_builder_ = std::make_shared<UniformDistributionBuilder>();
    MutableDistributionBuilderPtr wave_dist_builder_ =
        std::make_shared<ZipfianDistributionBuilder>();

    DataMapBuilderPtr data_map_builder_ = std::make_shared<ArrayDataMapBuilder>();

public:
    double get_creakers_size() const {
        return creakers_size_;
    }

    double get_wave_size() const {
        return wave_size_;
    }

    DataMapBuilderPtr get_data_map_builder() const {
        return data_map_builder_;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_creakers_size(double creakers_size) {
        creakers_size_ = creakers_size;
        return *this;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_creakers_ratio(double creakers_ratio) {
        creakers_ratio_ = creakers_ratio;
        return *this;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_wave_size(double wave_size) {
        wave_size_ = wave_size;
        return *this;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_creakers_dist_builder(
        DistributionBuilderPtr creakers_dist_builder) {
        creakers_dist_builder_ = std::move(creakers_dist_builder);
        return *this;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_wave_dist_builder(
        MutableDistributionBuilderPtr wave_dist_builder) {
        wave_dist_builder_ = std::move(wave_dist_builder);
        return *this;
    }

    CreakersAndWaveArgsGeneratorBuilder& set_data_map_builder(DataMapBuilderPtr data_map_builder) {
        data_map_builder_ = std::move(data_map_builder);
        return *this;
    }

    ArgsGeneratorBuilder& init(size_t range) override {
        creakers_length_ = range * creakers_size_;
        creakers_begin_ = range - creakers_length_;
        start_wave_length_ = range * wave_size_;
        wave_end_ = creakers_begin_;
        wave_begin_ = wave_end_ - start_wave_length_;
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_shared<CreakersAndWaveArgsGenerator>(
            rng, creakers_ratio_, creakers_begin_, wave_begin_, wave_end_,
            creakers_dist_builder_->build(rng, creakers_length_), wave_dist_builder_->build(rng),
            data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "CreakersAndWaveArgsGeneratorBuilder";
        j["creakersRatio"] = creakers_ratio_;
        j["creakersSize"] = creakers_size_;
        j["waveSize"] = wave_size_;
        j["creakersDistBuilder"] = *creakers_dist_builder_;
        j["waveDistBuilder"] = *wave_dist_builder_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        creakers_ratio_ = j["creakersRatio"];
        creakers_size_ = j["creakersSize"];
        wave_size_ = j["waveSize"];
        creakers_dist_builder_ = get_distribution_from_json(j["creakersDistBuilder"]);
        wave_dist_builder_ = get_mutable_distribution_from_json(j["waveDistBuilder"]);
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE", indents) +
               indented_title_with_data("Creakers Ratio", creakers_ratio_, indents) +
               indented_title_with_data("Creakers Size", creakers_size_, indents) +
               indented_title_with_data("Wave Size", wave_size_, indents) +
               indented_title("Creakers Distribution", indents) +
               creakers_dist_builder_->to_string(indents + 1) +
               indented_title("Wave Distribution", indents) +
               wave_dist_builder_->to_string(indents + 1) + indented_title("Data Map", indents) +
               data_map_builder_->to_string(indents + 1);
    }

    ~CreakersAndWaveArgsGeneratorBuilder() override = default;
};

class CreakersAndWavePrefillArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    size_t wave_begin_;
    size_t prefill_length_;
    PAD;

    double creakers_size_ = 0;
    double wave_size_ = 0;

    DataMapBuilderPtr data_map_builder_ = std::make_shared<ArrayDataMapBuilder>();

public:
    size_t get_prefill_length() const {
        return prefill_length_;
    }

    size_t get_prefill_length(size_t range) const {
        return range * creakers_size_ + range * wave_size_;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder() {
    }

    CreakersAndWavePrefillArgsGeneratorBuilder& set_parameters_by_builder(
        const CreakersAndWaveArgsGeneratorBuilder& builder) {
        creakers_size_ = builder.get_creakers_size();
        wave_size_ = builder.get_wave_size();
        data_map_builder_ = builder.get_data_map_builder();
        return *this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder& set_creakers_size(double creakers_size) {
        creakers_size_ = creakers_size;
        return *this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder& set_wave_size(double wave_size) {
        wave_size_ = wave_size;
        return *this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder& set_data_map_builder(
        DataMapBuilderPtr data_map_builder) {
        data_map_builder_ = data_map_builder;
        return *this;
    }

    CreakersAndWavePrefillArgsGeneratorBuilder& init(size_t range) override {
        prefill_length_ = range * creakers_size_ + range * wave_size_;
        wave_begin_ = range - prefill_length_;
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_shared<CreakersAndWavePrefillArgsGenerator>(
            rng, wave_begin_, prefill_length_, data_map_builder_->build());
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "CreakersAndWavePrefillArgsGeneratorBuilder";
        j["creakersSize"] = creakers_size_;
        j["waveSize"] = wave_size_;
        j["dataMapBuilder"] = *data_map_builder_;
    }

    void from_json(const nlohmann::json& j) override {
        creakers_size_ = j["creakersSize"];
        wave_size_ = j["waveSize"];
        data_map_builder_ = get_data_map_from_json(j["dataMapBuilder"]);
    }

    std::string to_string(size_t indents) override {
        return indented_title_with_str_data("Type", "CREAKERS_AND_WAVE_PREFILL", indents) +
               indented_title_with_data("Creakers Size", creakers_size_, indents) +
               indented_title_with_data("Wave Size", wave_size_, indents) +
               indented_title("Data Map", indents) + data_map_builder_->to_string(indents + 1);
    }

    ~CreakersAndWavePrefillArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
