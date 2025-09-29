#include <cstdio>
#include <iostream>
#include <memory>
#include "parse_argument.h"
#include "plaf.h"
#include "common/server_clock.h"
#include "workloads/bench_parameters.h"


int main(int argc, char** argv) {
    printUptimeStampForPERF("MAIN_START");

    std::cout << "binary=" << argv[0] << std::endl;

    std::shared_ptr<BenchParameters> bench_parameters = std::make_shared<BenchParameters>();
    std::shared_ptr<Parameters> prefill = nullptr;
    std::shared_ptr<Parameters> warm_up = nullptr;
    std::shared_ptr<Parameters> test = nullptr;

    ParseArgument args = ParseArgument(argc, argv).next();
    bool detail_stats = false;
    bool create_default_prefill = false;
    bool result_statistic_to_file = false;
    std::string result_statistic_file_name;


}
