#pragma once

#include "workloads/args_generators/args_generator.h"

template <typename K>
class NullArgsGenerator : public ArgsGenerator<K> {
public:
    NullArgsGenerator() = default;

    K nextGet() {
        setbench_error("Operation not supported");
    }

    K nextInsert() {
        setbench_error("Operation not supported");
    }

    K nextRemove() {
        setbench_error("Operation not supported");
    }

    std::pair<K, K> nextRange() {
        setbench_error("Operation not supported");
    }

    ~NullArgsGenerator() = default;
};

#include "workloads/args_generators/args_generator_builder.h"
#include "globals_extern.h"

// template<typename K>
class NullArgsGeneratorBuilder : public ArgsGeneratorBuilder {
public:
    NullArgsGeneratorBuilder* init(size_t _range) override {
        //        dataMapBuilder->init(_range);
        return this;
    }

    NullArgsGenerator<K>* build(Random64& _rng) override {
        return new NullArgsGenerator<K>();
    }

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "NullArgsGeneratorBuilder";
    }

    void fromJson(const nlohmann::json& j) override {
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "NULL", indents);
        return res;
    }

    ~NullArgsGeneratorBuilder() override = default;
};
