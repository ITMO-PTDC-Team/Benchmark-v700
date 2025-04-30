#include "generalized_args_generator.h"

void GeneralizedArgsGeneratorBuilder::fromJson(const nlohmann::json &j) {
    if (!j.contains("builders") || !j["builders"].is_array()) {
        setbench_error("Wrong number of parameters for ArgsGenerator");
        return;
    }

    for (const auto& item : j["builders"]) {

        if (!item.contains("opers") || !item.contains("argsGeneratorBuilder")) {
            setbench_error("Missing required fields in argsGeneratorBuilder item");
            continue;
        }

        const auto& operationsList = item["opers"];
        std::shared_ptr<ArgsGeneratorBuilder> aux;
        aux.reset(getArgsGeneratorFromJson(item["argsGeneratorBuilder"]));
        aux->init(_range);
        for (const auto& opName : operationsList) {
            if (opName == "get") {
                getArgsGeneratorBuilder = aux;
            } else if (opName == "insert") {
                insertArgsGeneratorBuilder = aux;
            } else if (opName == "remove") {
                removeArgsGeneratorBuilder = aux;
            } else if (opName == "rangeQuery") {
                rangeQueryArgsGeneratorBuilder = aux;
            } else {
                setbench_error("Wrong operation type to support");
            }
        }
        aux.reset();
    }
}