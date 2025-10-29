//
// Created by Ravil Galiev on 31.07.2023.
//
#pragma once

#include "distribution_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "errors.h"

DistributionBuilder* get_distribution_from_json(const nlohmann::json& j) {
    std::string class_name = j["ClassName"];
    DistributionBuilder* distribution_builder;
    if (class_name == "UniformDistributionBuilder") {
        distribution_builder = new UniformDistributionBuilder();
    } else if (class_name == "ZipfianDistributionBuilder") {
        distribution_builder = new ZipfianDistributionBuilder();
    } else if (class_name == "SkewedUniformDistributionBuilder") {
        distribution_builder = new SkewedUniformDistributionBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name DistributionBuilder -- " + class_name)
    }

    distribution_builder->from_json(j);
    return distribution_builder;
}

MutableDistributionBuilder* get_mutable_distribution_from_json(const nlohmann::json& j) {
    return dynamic_cast<MutableDistributionBuilder*>(get_distribution_from_json(j));
}
