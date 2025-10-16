#pragma once

#include <vector>
#include <utility>
#include <map>

#include "workloads/args_generators/args_generator.h"

template <typename K>
class GeneralizedArgsGenerator : public ArgsGenerator<K> {
private:
    std::shared_ptr<ArgsGenerator<K>> _get_generator;
    std::shared_ptr<ArgsGenerator<K>> _insert_generator;
    std::shared_ptr<ArgsGenerator<K>> _remove_generator;
    std::shared_ptr<ArgsGenerator<K>> _range_generator;

public:
    GeneralizedArgsGenerator(std::shared_ptr<ArgsGenerator<K>> get_gen,
                             std::shared_ptr<ArgsGenerator<K>> insert_gen,
                             std::shared_ptr<ArgsGenerator<K>> remove_gen,
                             std::shared_ptr<ArgsGenerator<K>> range_gen)
        : _get_generator(get_gen),
          _insert_generator(insert_gen),
          _remove_generator(remove_gen),
          _range_generator(range_gen) {
    }

    K nextGet() override {
        return _get_generator->nextGet();
    }

    K nextInsert() override {
        return _insert_generator->nextInsert();
    }

    K nextRemove() override {
        return _remove_generator->nextRemove();
    }

    std::pair<K, K> nextRange() override {
        return _range_generator->nextRange();
    }

    ~GeneralizedArgsGenerator() = default;
};

#include "workloads/args_generators/args_generator_builder.h"
#include "null_args_generator.h"

#include "globals_extern.h"

#include <set>

static const std::set<std::string> oper_types{"get", "insert", "remove", "rangeQuery"};

ArgsGeneratorBuilder* getArgsGeneratorFromJson(const nlohmann::json& j);

// template<typename K>
class GeneralizedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
    std::vector<std::pair<std::vector<std::string>, std::shared_ptr<ArgsGeneratorBuilder>>>
        args_generator_builders;

    std::set<std::string> undec_oper_types = oper_types;

public:
    GeneralizedArgsGeneratorBuilder() = default;

    GeneralizedArgsGeneratorBuilder* addArgsGeneratorBuilder(
        const std::vector<std::string>& opers, ArgsGeneratorBuilder* _argsGenBuilder) {
        for (auto oper_type : opers) {
            if (oper_types.find(oper_type) == oper_types.end()) {
                setbench_error("Unsupported operation type: " + oper_type);
            }
            if (undec_oper_types.find(oper_type) == undec_oper_types.end()) {
                setbench_error("Multiple declaration of operation type: " + oper_type);
            }
            std::cout << undec_oper_types.erase(oper_type);
        }

        std::shared_ptr<ArgsGeneratorBuilder> new_builder;
        new_builder.reset(_argsGenBuilder);
        args_generator_builders.push_back({opers, new_builder});

        return this;
    }

    GeneralizedArgsGeneratorBuilder* init(size_t range) override {
        if (!undec_oper_types.empty()) {
            addArgsGeneratorBuilder(
                std::vector<std::string>(undec_oper_types.begin(), undec_oper_types.end()),
                new NullArgsGeneratorBuilder());
        }

        for (auto& it : args_generator_builders) {
            it.second->init(range);
        }
        return this;
    }

    GeneralizedArgsGenerator<K>* build(Random64& _rng) override {
        std::map<std::string, std::shared_ptr<ArgsGenerator<K>>> built;
        for (auto& it : args_generator_builders) {
            std::shared_ptr<ArgsGenerator<K>> u;
            u.reset(it.second->build(_rng));
            for (auto& oper_type : it.first) {
                built.insert({oper_type, u});
            }
        }

        return new GeneralizedArgsGenerator<K>(std::move(built["get"]), std::move(built["insert"]),
                                               std::move(built["remove"]),
                                               std::move(built["rangeQuery"]));
    }

    void toJson(nlohmann::json& j) const override {
        j["ClassName"] = "GeneralizedArgsGeneratorBuilder";
        nlohmann::json builders = nlohmann::json::array();
        for (size_t id = 0; id < args_generator_builders.size(); ++id) {
            nlohmann::json current_builder;

            nlohmann::json opers = nlohmann::json::array();
            for (auto& oper_name : args_generator_builders[id].first) {
                opers.push_back(oper_name);
            }
            current_builder["opers"] = opers;
            current_builder["argsGeneratorBuilder"] = *(args_generator_builders[id].second);

            builders.push_back(current_builder);
        }
        j["builders"] = builders;
    }

    void fromJson(const nlohmann::json& j) override {
        if (!j.contains("builders") || !j["builders"].is_array()) {
            setbench_error("Wrong number of parameters for ArgsGenerator");
        }

        for (const auto& item : j["builders"]) {
            if (!item.contains("opers") || !item.contains("argsGeneratorBuilder")) {
                setbench_error("Missing required fields in argsGeneratorBuilder item");
            }

            addArgsGeneratorBuilder(item["opers"],
                                    getArgsGeneratorFromJson(item["argsGeneratorBuilder"]));
        }
    }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "GENERALIZED", indents);
        for (auto& it : args_generator_builders) {
            std::string opers = it.first[0];
            for (size_t i = 1; i < it.first.size(); i++) {
                opers += ", " + it.first[i];
            }

            res += indented_title("Args Generators (" + opers + ")", indents);
            res += it.second->toString(indents + 1);
        }
        return res;
    }

    ~GeneralizedArgsGeneratorBuilder() override = default;
};
