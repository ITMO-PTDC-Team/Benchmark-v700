#ifndef SETBENCH_GENERALIZED_ARGS_GENERATOR_IMPL_H
#define SETBENCH_GENERALIZED_ARGS_GENERATOR_IMPL_H

#include "generalized_args_generator.h"
#include "workloads/args_generators/args_generator_json_convector.h"

void GeneralizedArgsGeneratorBuilder::fromJson(const nlohmann::json &j) {
    if (!j.contains("builders") || !j["builders"].is_array()) {
        setbench_error("Wrong number of parameters for ArgsGenerator");
        return;
    }

    size_t current_id = 0;
    for (const auto& item : j["builders"]) {

        if (!item.contains("opers") || !item.contains("argsGeneratorBuilder")) {
            setbench_error("Missing required fields in argsGeneratorBuilder item");
            continue;
        }

        if (item.contains("id")) {
            _ids.push_back(item["id"]); 
        } else {
            _ids.push_back(current_id++);
        }

        const auto& operationsList = item["opers"];
        std::shared_ptr<ArgsGeneratorBuilder> aux;
        aux.reset(getArgsGeneratorFromJson(item["argsGeneratorBuilder"]));
        std::for_each(operationsList.begin(), operationsList.end(), [](const std::string& oper_type) {
            if (oper_type != "get" && oper_type != "insert" && oper_type != "remove" && oper_type != "rangeQuery") {
                setbench_error("Wrong operation type to support");
            }
        });
        
        args_generator_builders.push_back(
            std::make_pair(operationsList, aux)
        );
    }
}

#endif //SETBENCH_GENERALIZED_ARGS_GENERATOR_IMPL_H