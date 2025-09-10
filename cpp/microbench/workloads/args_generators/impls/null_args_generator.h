#ifndef SETBENCH_NULL_ARGS_GENERATOR_H
#define SETBENCH_NULL_ARGS_GENERATOR_H

#include "workloads/args_generators/args_generator.h"

// template<typename size_t>
class NullArgsGenerator : public ArgsGenerator {
public:
    NullArgsGenerator() = default;

    size_t nextGet() {
        setbench_error("Operation not supported");
    }

    size_t nextInsert() {
        setbench_error("Operation not supported");
    }

    size_t nextRemove() {
        setbench_error("Operation not supported");
    }

    std::pair<size_t, size_t> nextRange() {
        setbench_error("Operation not supported");
    }

    std::vector<shared_ptr<DataMap>> getInternalDataMaps() {
        std::vector<std::shared_ptr<DataMap>> result;
        result.reserve(4);
        for (int i = 0; i<4; ++i) {
            result.emplace_back(nullptr);
        }
        return result;
    }

    ~NullArgsGenerator() = default;
};

#include "workloads/args_generators/args_generator_builder.h"
#include "globals_extern.h"

//template<typename size_t>
class NullArgsGeneratorBuilder : public ArgsGeneratorBuilder {
public:
    NullArgsGeneratorBuilder *init(size_t _range) override {
//        dataMapBuilder->init(_range);
        return this;
    }

    NullArgsGenerator *build(Random64 &_rng) override {
        return new NullArgsGenerator();
    }

    void toJson(nlohmann::json &j) const override {
        j["ClassName"] = "NullArgsGeneratorBuilder";
    }

    void fromJson(const nlohmann::json &j) override { }

    std::string toString(size_t indents = 1) override {
        std::string res;
        res += indented_title_with_str_data("Type", "NULL", indents);
        return res;
    }

    ~NullArgsGeneratorBuilder() override = default;
};


#endif //SETBENCH_NULL_ARGS_GENERATOR_H
