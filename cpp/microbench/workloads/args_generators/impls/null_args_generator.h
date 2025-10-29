#pragma once

#include "workloads/args_generators/args_generator.h"

namespace microbench::workload {

template <typename K>
class NullArgsGenerator : public ArgsGenerator<K> {
public:
    NullArgsGenerator() = default;

    K next_get() {
        setbench_error("Operation not supported");
    }

    K next_insert() {
        setbench_error("Operation not supported");
    }

    K next_remove() {
        setbench_error("Operation not supported");
    }

    std::pair<K, K> next_range() {
        setbench_error("Operation not supported");
    }

    ~NullArgsGenerator() = default;
};

}  // namespace microbench::workload

#include "workloads/args_generators/args_generator_builder.h"
#include "globals_extern.h"

namespace microbench::workload {

class NullArgsGeneratorBuilder : public ArgsGeneratorBuilder {
public:
    NullArgsGeneratorBuilder* init(size_t range) override {
        //        dataMapBuilder->init(_range);
        return this;
    }

    NullArgsGenerator<K>* build(Random64& rng) override {
        return new NullArgsGenerator<K>();
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "NullArgsGeneratorBuilder";
    }

    void from_json(const nlohmann::json& j) override {
    }

    std::string to_string(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "NULL", indents);
        return res;
    }

    ~NullArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
