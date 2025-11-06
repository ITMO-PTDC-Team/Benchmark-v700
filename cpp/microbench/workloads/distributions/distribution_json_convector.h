//
// Created by Ravil Galiev on 31.07.2023.
//

#ifndef SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H
#define SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H

#include "distribution_builder.h"
#include "workloads/distributions/builders/uniform_distribution_builder.h"
#include "workloads/distributions/builders/zipfian_distribution_builder.h"
#include "workloads/distributions/builders/skewed_uniform_distribution_builder.h"
#include "errors.h"

DistributionBuilder *getDistributionFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    DistributionBuilder *distributionBuilder;
    if (className == "UniformDistributionBuilder") {
        distributionBuilder = new UniformDistributionBuilder();
    } else if (className == "ZipfianDistributionBuilder") {
        distributionBuilder = new ZipfianDistributionBuilder();
    } else if (className == "SkewedUniformDistributionBuilder") {
        distributionBuilder = new SkewedUniformDistributionBuilder();
    } else {
        setbench_error("JSON PARSER: Unknown class name DistributionBuilder -- " + className)
    }

    distributionBuilder->fromJson(j);
    return distributionBuilder;
}

MutableDistributionBuilder *getMutableDistributionFromJson(const nlohmann::json &j) {
    return dynamic_cast<MutableDistributionBuilder *>(getDistributionFromJson(j));
}

#endif //SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H
