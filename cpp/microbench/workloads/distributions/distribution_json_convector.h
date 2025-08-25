//
// Created by Ravil Galiev on 31.07.2023.
//

#ifndef SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H
#define SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H

#include <memory>
#include "distribution_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "errors.h"

std::shared_ptr<DistributionBuilder> getDistributionFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    std::shared_ptr<DistributionBuilder> distributionBuilder;
    if (className == "UniformDistributionBuilder") {
        distributionBuilder = std::make_shared<UniformDistributionBuilder>();
    } else if (className == "ZipfianDistributionBuilder") {
        distributionBuilder = std::make_shared<ZipfianDistributionBuilder>();
    } else if (className == "SkewedUniformDistributionBuilder") {
        distributionBuilder = std::make_shared<SkewedUniformDistributionBuilder>();
    } else {
        setbench_error("JSON PARSER: Unknown class name DistributionBuilder -- " + className)
    }

    distributionBuilder->fromJson(j);
    return distributionBuilder;
}

std::shared_ptr<MutableDistributionBuilder> getMutableDistributionFromJson(const nlohmann::json &j) {
    return std::dynamic_pointer_cast<MutableDistributionBuilder>(getDistributionFromJson(j));
}

#endif //SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H
