#pragma once

#include <memory>
#include <utility>

#include "workloads/args_generators/args_generator.h"

namespace microbench::workload {

using KeyType = int64_t;

class GeneralizedArgsGenerator : public ArgsGenerator {
private:
    ArgsGeneratorPtr get_generator_;
    ArgsGeneratorPtr insert_generator_;
    ArgsGeneratorPtr remove_generator_;
    ArgsGeneratorPtr range_generator_;

public:
    GeneralizedArgsGenerator(ArgsGeneratorPtr get_gen,
                             ArgsGeneratorPtr insert_gen,
                             ArgsGeneratorPtr remove_gen,
                             ArgsGeneratorPtr range_gen)
        : get_generator_(std::move(get_gen)),
          insert_generator_(std::move(insert_gen)),
          remove_generator_(std::move(remove_gen)),
          range_generator_(std::move(range_gen)) {
    }

    KeyType next_get() override {
        return get_generator_->next_get();
    }

    KeyType next_insert() override {
        return insert_generator_->next_insert();
    }

    KeyType next_remove() override {
        return remove_generator_->next_remove();
    }

    std::pair<KeyType, KeyType> next_range() override {
        return range_generator_->next_range();
    }

    ~GeneralizedArgsGenerator() = default;
};

}  // namespace microbench::workload
