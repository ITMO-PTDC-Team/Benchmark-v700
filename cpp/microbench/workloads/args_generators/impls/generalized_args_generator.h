#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <map>

#include "workloads/args_generators/args_generator.h"

namespace microbench::workload {

template <typename K>
class GeneralizedArgsGenerator : public ArgsGenerator<K> {
private:
    std::shared_ptr<ArgsGenerator<K>> get_generator_;
    std::shared_ptr<ArgsGenerator<K>> insert_generator_;
    std::shared_ptr<ArgsGenerator<K>> remove_generator_;
    std::shared_ptr<ArgsGenerator<K>> range_generator_;

public:
    GeneralizedArgsGenerator(std::shared_ptr<ArgsGenerator<K>> get_gen,
                             std::shared_ptr<ArgsGenerator<K>> insert_gen,
                             std::shared_ptr<ArgsGenerator<K>> remove_gen,
                             std::shared_ptr<ArgsGenerator<K>> range_gen)
        : get_generator_(get_gen),
          insert_generator_(insert_gen),
          remove_generator_(remove_gen),
          range_generator_(range_gen) {
    }

    K next_get() override {
        return get_generator_->next_get();
    }

    K next_insert() override {
        return insert_generator_->next_insert();
    }

    K next_remove() override {
        return remove_generator_->next_remove();
    }

    std::pair<K, K> next_range() override {
        return range_generator_->next_range();
    }

    ~GeneralizedArgsGenerator() = default;
};

}  // namespace microbench::workload

#include "workloads/args_generators/args_generator_builder.h"
#include "null_args_generator.h"

#include "globals_extern.h"

#include <set>

namespace microbench::workload {

static const std::set<std::string> OPER_TYPES{"get", "insert", "remove", "rangeQuery"};

ArgsGeneratorBuilder* get_args_generator_from_json(const nlohmann::json& j);

// template<typename K>
class GeneralizedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    std::vector<std::pair<std::vector<std::string>, std::shared_ptr<ArgsGeneratorBuilder>>>
        args_generator_builders_;

    std::set<std::string> undec_oper_types_ = OPER_TYPES;

public:
    GeneralizedArgsGeneratorBuilder() = default;

    GeneralizedArgsGeneratorBuilder* add_args_generator_builder(
        const std::vector<std::string>& opers, ArgsGeneratorBuilder* args_gen_builder) {
        for (auto oper_type : opers) {
            if (OPER_TYPES.find(oper_type) == OPER_TYPES.end()) {
                setbench_error("Unsupported operation type: " + oper_type);
            }
            if (undec_oper_types_.find(oper_type) == undec_oper_types_.end()) {
                setbench_error("Multiple declaration of operation type: " + oper_type);
            }
            std::cout << undec_oper_types_.erase(oper_type);
        }

        std::shared_ptr<ArgsGeneratorBuilder> new_builder;
        new_builder.reset(args_gen_builder);
        args_generator_builders_.push_back({opers, new_builder});

        return this;
    }

    GeneralizedArgsGeneratorBuilder* init(size_t range) override {
        if (!undec_oper_types_.empty()) {
            add_args_generator_builder(
                std::vector<std::string>(undec_oper_types_.begin(), undec_oper_types_.end()),
                new NullArgsGeneratorBuilder());
        }

        for (auto& it : args_generator_builders_) {
            it.second->init(range);
        }
        return this;
    }

    GeneralizedArgsGenerator<K>* build(Random64& rng) override {
        std::map<std::string, std::shared_ptr<ArgsGenerator<K>>> built;
        for (auto& it : args_generator_builders_) {
            std::shared_ptr<ArgsGenerator<K>> u;
            u.reset(it.second->build(rng));
            for (auto& oper_type : it.first) {
                built.insert({oper_type, u});
            }
        }

        return new GeneralizedArgsGenerator<K>(std::move(built["get"]), std::move(built["insert"]),
                                               std::move(built["remove"]),
                                               std::move(built["rangeQuery"]));
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
