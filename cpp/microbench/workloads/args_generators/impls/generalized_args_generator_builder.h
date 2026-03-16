#pragma once

#include "args_generators/args_generator.h"
#include "args_generators/impls/generalized_args_generator.h"
#include "args_generators/impls/null_args_generator_builder.h"
#include "errors.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "globals_extern.h"
#include <memory>
#include <set>
#include <string>

namespace microbench::workload {

ArgsGeneratorBuilderPtr get_args_generator_from_json(const nlohmann::json& j);

static const std::set<std::string> OPER_TYPES{"get", "insert", "remove", "rangeQuery"};

class GeneralizedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    std::vector<std::pair<std::vector<std::string>, ArgsGeneratorBuilderPtr>>
        args_generator_builders_;

    std::set<std::string> undec_oper_types_ = OPER_TYPES;

public:
    GeneralizedArgsGeneratorBuilder() = default;

    GeneralizedArgsGeneratorBuilder& add_args_generator_builder(
        const std::vector<std::string>& opers, ArgsGeneratorBuilderPtr args_gen_builder) {
        for (auto oper_type : opers) {
            if (OPER_TYPES.find(oper_type) == OPER_TYPES.end()) {
                setbench_error("Unsupported operation type: " + oper_type);
            }
            if (undec_oper_types_.find(oper_type) == undec_oper_types_.end()) {
                setbench_error("Multiple declaration of operation type: " + oper_type);
            }
            std::cout << undec_oper_types_.erase(oper_type);
        }

        args_generator_builders_.emplace_back(opers, std::move(args_gen_builder));

        return *this;
    }

    GeneralizedArgsGeneratorBuilder& init(size_t range) override {
        if (!undec_oper_types_.empty()) {
            add_args_generator_builder(
                std::vector<std::string>(undec_oper_types_.begin(), undec_oper_types_.end()),
                std::make_unique<NullArgsGeneratorBuilder>());
        }

        for (auto& it : args_generator_builders_) {
            it.second->init(range);
        }
        return *this;
    }

    ArgsGeneratorPtr build(Random64& rng) override {
        std::map<std::string, ArgsGeneratorPtr> built;
        for (auto& it : args_generator_builders_) {
            for (auto& oper_type : it.first) {
                built.emplace(oper_type, it.second->build(rng));
            }
        }

        return ArgsGeneratorPtr(new GeneralizedArgsGenerator(
            std::move(built["get"]), std::move(built["insert"]), std::move(built["remove"]),
            std::move(built["rangeQuery"])));
    }

    void to_json(nlohmann::json& j) const override {
        j["ClassName"] = "GeneralizedArgsGeneratorBuilder";
        nlohmann::json builders = nlohmann::json::array();
        for (size_t id = 0; id < args_generator_builders_.size(); ++id) {
            nlohmann::json current_builder;

            nlohmann::json opers = nlohmann::json::array();
            for (auto& oper_name : args_generator_builders_[id].first) {
                opers.push_back(oper_name);
            }
            current_builder["opers"] = opers;
            current_builder["argsGeneratorBuilder"] = *(args_generator_builders_[id].second);

            builders.push_back(current_builder);
        }
        j["builders"] = builders;
    }

    void from_json(const nlohmann::json& j) override {
        if (!j.contains("builders") || !j["builders"].is_array()) {
            setbench_error("Wrong number of parameters for ArgsGenerator");
        }

        for (const auto& item : j["builders"]) {
            if (!item.contains("opers") || !item.contains("argsGeneratorBuilder")) {
                setbench_error("Missing required fields in argsGeneratorBuilder item");
            }

            add_args_generator_builder(item["opers"],
                                       get_args_generator_from_json(item["argsGeneratorBuilder"]));
        }
    }

    std::string to_string(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "GENERALIZED", indents);
        for (auto& it : args_generator_builders_) {
            std::string opers = it.first[0];
            for (size_t i = 1; i < it.first.size(); i++) {
                opers += ", " + it.first[i];
            }

            res += indented_title("Args Generators (" + opers + ")", indents);
            res += it.second->to_string(indents + 1);
        }
        return res;
    }

    ~GeneralizedArgsGeneratorBuilder() override = default;
};

}  // namespace microbench::workload
