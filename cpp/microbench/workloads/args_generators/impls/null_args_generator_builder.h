#pragma once

#include <memory>
#include "args_generators/args_generator.h"
#include "workloads/args_generators/impls/null_args_generator.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "globals_extern.h"

namespace microbench::workload {

class NullArgsGeneratorBuilder : public ArgsGeneratorBuilder {
public:
    NullArgsGeneratorBuilder& init(size_t range) override {
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        return std::make_shared<NullArgsGenerator>();
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
