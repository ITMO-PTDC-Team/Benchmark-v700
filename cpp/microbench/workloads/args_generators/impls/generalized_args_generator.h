#ifndef SETBENCH_GENERALIZED_ARGS_GENERATOR_H
#define SETBENCH_GENERALIZED_ARGS_GENERATOR_H

#include <vector>
#include <utility>
#include <map>

#include "workloads/args_generators/args_generator.h"
#include "workloads/distributions/distribution.h"
#include "workloads/data_maps/data_map.h"

template<typename K>
class GeneralizedArgsGenerator : public ArgsGenerator<K> {
private:
    std::shared_ptr<ArgsGenerator<K>> _get_generator;
    std::shared_ptr<ArgsGenerator<K>> _insert_generator;
    std::shared_ptr<ArgsGenerator<K>> _remove_generator;
    std::shared_ptr<ArgsGenerator<K>> _range_generator;

public:
    GeneralizedArgsGenerator(std::shared_ptr<ArgsGenerator<K>>& get_gen, 
                             std::shared_ptr<ArgsGenerator<K>>& insert_gen,
                             std::shared_ptr<ArgsGenerator<K>>& remove_gen,
                             std::shared_ptr<ArgsGenerator<K>>& range_gen)
            : _get_generator(get_gen), _insert_generator(insert_gen), _remove_generator(remove_gen), _range_generator(range_gen) {}


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

#include "workloads/data_maps/data_map_builder.h"
#include "workloads/data_maps/builders/id_data_map_builder.h"
#include "workloads/args_generators/args_generator_builder.h"
#include "workloads/distributions/distribution_json_convector.h"
#include "workloads/data_maps/data_map_json_convector.h"
#include "globals_extern.h"

//template<typename K>
class GeneralizedArgsGeneratorBuilder : public ArgsGeneratorBuilder {
private:
    size_t _range;
    std::vector<size_t> _ids;
    std::vector<
        std::pair<std::vector<std::string>,
                  std::shared_ptr<ArgsGeneratorBuilder>
                  >> args_generator_builders;
public:
    GeneralizedArgsGeneratorBuilder() = default;

    GeneralizedArgsGeneratorBuilder *init(size_t range) override {
        _range = _range;
        for (auto& current_pair : args_generator_builders) {
            current_pair.second->init(range);
        };
        return this;
    }

    GeneralizedArgsGeneratorBuilder *addGeneratorBuilder(const std::vector<std::string>& opers, 
                                                         ArgsGeneratorBuilder *_distributionBuilder,
                                                        size_t id = 1) {
        std::shared_ptr<ArgsGeneratorBuilder> new_builder;
        new_builder.reset(_distributionBuilder);
        args_generator_builders.push_back({opers, new_builder});
        _ids.push_back(id);
        return this;
    }

    GeneralizedArgsGenerator<K> *build(Random64 &_rng) override {
        std::map<std::string, std::shared_ptr<ArgsGenerator<K>>> builded;
        for (auto& current_pair : args_generator_builders) {
            std::shared_ptr<ArgsGenerator<K>> u;
            u.reset(current_pair.second->build(_rng));
            for (auto& oper_type : current_pair.first) {
                builded.insert(std::make_pair(oper_type, u));
            }
        };
        return new GeneralizedArgsGenerator<K>(builded["get"],
                                               builded["insert"],
                                               builded["remove"],
                                               builded["rangeQuery"]);
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "GeneralizedArgsGeneratorBuilder";
        nlohmann::json builders = nlohmann::json::array();
        size_t current_id = 0;
        for (size_t id = 0; id<args_generator_builders.size(); ++id) {
            nlohmann::json current_builder;

            nlohmann::json opers = nlohmann::json::array();
            for (auto& oper_name : args_generator_builders[id].first) {
                opers.push_back(oper_name);
            }
            current_builder["opers"] = opers;
            current_builder["id"] = _ids[id];
            current_builder["argsGeneratorBuilder"] = *(args_generator_builders[id].second);
            
            builders.push_back(current_builder);
        };
        j["builders"] = builders;
    }

    void fromJson(const nlohmann::json &j) override;

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "GENERALIZED", indents);
        size_t current_id = 0;
        for (auto& current_pair : args_generator_builders) {
            for (auto& oper_type : current_pair.first) {
                res += indented_title_with_str_data("ArgsGeneratorBuilder", "Type: " + oper_type, indents);
                res += indented_title_with_str_data("ArgsGeneratorID", std::to_string(_ids[current_id]), indents);
                res += current_pair.second->toString(indents + 1);
            }
            ++current_id;
        }
        return res;
    }

    ~GeneralizedArgsGeneratorBuilder() override = default;

};


#endif //SETBENCH_GENERALIZED_ARGS_GENERATOR_H
