//
// Created by Ravil Galiev on 31.07.2023.
//
#pragma once

#include "distributions/distribution_builder.h"
#include "nlohmann/json_fwd.hpp"

namespace microbench::workload {

DistributionBuilderPtr get_distribution_from_json(const nlohmann::json& j);

MutableDistributionBuilderPtr get_mutable_distribution_from_json(const nlohmann::json& j);

}  // namespace microbench::workload

#ifdef DISTR_CONV_IMPL

#include "distributions/builders/skewed_uniform_distribution_builder.h"
#include "distributions/builders/uniform_distribution_builder.h"
#include "distributions/builders/zipfian_distribution_builder.h"
#include "workloads/distributions/distribution_builder.h"
#include "errors.h"

namespace microbench::workload {

DistributionBuilderPtr get_distribution_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    DistributionBuilderPtr distribution_builder;
    if (class_name == "UniformDistributionBuilder") {
        distribution_builder = std::make_shared<UniformDistributionBuilder>();
    } else if (class_name == "ZipfianDistributionBuilder") {
        distribution_builder = std::make_shared<ZipfianDistributionBuilder>();
    } else if (class_name == "SkewedUniformDistributionBuilder") {
        distribution_builder = std::make_shared<SkewedUniformDistributionBuilder>();
    } else {
        setbench_error("JSON PARSER: Unknown class name DistributionBuilder -- " + class_name)
    }

    distribution_builder->from_json(j);
    return distribution_builder;
}

MutableDistributionBuilderPtr get_mutable_distribution_from_json(const nlohmann::json& j) {
    return std::dynamic_pointer_cast<MutableDistributionBuilder>(get_distribution_from_json(j));
}

}  // namespace microbench::workload

#endif
