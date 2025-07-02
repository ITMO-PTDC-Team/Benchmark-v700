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

class BaseDistributionBuilderFactory {
    public:
        virtual ~BaseDistributionBuilderFactory() = default;
        virtual DistributionBuilder* create() = 0;
};
    
template <typename DistributionBuilder>
class DistributionBuilderFactory : public BaseDistributionBuilderFactory {
    public:
    DistributionBuilder *create() override {
        return new DistributionBuilder();
    }
};

#define REGISTER_DISTRIBUTION_BUILDER(className) \
    map.insert({#className, std::make_unique<DistributionBuilderFactory<className>>()})
    
inline static std::map<std::string, std::unique_ptr<BaseDistributionBuilderFactory>> distributionFactoryMap = [] {
    std::map<std::string, std::unique_ptr<BaseDistributionBuilderFactory>> map;
    REGISTER_DISTRIBUTION_BUILDER(UniformDistributionBuilder);
    REGISTER_DISTRIBUTION_BUILDER(ZipfianDistributionBuilder);
    REGISTER_DISTRIBUTION_BUILDER(SkewedUniformDistributionBuilder);
    return map;
}();

DistributionBuilder *getDistributionFromJson(const nlohmann::json &j) {
    std::string className = j["ClassName"];
    if (distributionFactoryMap.find(className) != distributionFactoryMap.end()) {
        DistributionBuilder *distributionBuilder = distributionFactoryMap[className]->create();
        distributionBuilder->fromJson(j);
        return distributionBuilder;
    }
    setbench_error("JSON PARSER: Unknown class name DistributionBuilder -- " + className)
}

MutableDistributionBuilder *getMutableDistributionFromJson(const nlohmann::json &j) {
    return dynamic_cast<MutableDistributionBuilder *>(getDistributionFromJson(j));
}

#endif //SETBENCH_DISTRIBUTION_JSON_CONVECTOR_H
