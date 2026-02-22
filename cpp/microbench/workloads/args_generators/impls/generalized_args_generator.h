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
