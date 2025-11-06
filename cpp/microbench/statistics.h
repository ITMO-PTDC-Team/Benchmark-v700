//
// Created by Ravil Galiev on 03.08.2023.
//

#ifndef SETBENCH_STATISTICS_H
#define SETBENCH_STATISTICS_H

#include "define_global_statistics.h"
#include "gstats_global.h"
#include "debugprinting.h"
#include "json/single_include/nlohmann/json.hpp"
#include "globals_extern.h"

struct Statistic {
    long long totalAll = 0;

    long long totalGets;
    long long totalRQs;
    long long totalQueries;
    long long totalInserts;
    long long totalRemoves;
    long long totalUpdates;

    long long totalSuccessfulGets;
    long long totalSuccessfulInserts;
    long long totalSuccessfulRemoves;
    long long totalSuccessfulUpdates;

    long long totalFailGets;
    long long totalFailInserts;
    long long totalFailRemoves;
    long long totalFailUpdates;

    double SECONDS_TO_RUN;
    long long throughputSearches;
    long long throughputRQs;
    long long throughputQueries;
    long long throughputUpdates;
    long long throughputAll;

    Statistic(double _SECONDS_TO_RUN) {
        totalGets = GSTATS_GET_STAT_METRICS(num_searches, TOTAL)[0].sum;
        totalRQs = GSTATS_GET_STAT_METRICS(num_rq, TOTAL)[0].sum;
        totalQueries = totalGets + totalRQs;
        totalInserts = GSTATS_GET_STAT_METRICS(num_inserts, TOTAL)[0].sum;
        totalRemoves = GSTATS_GET_STAT_METRICS(num_removes, TOTAL)[0].sum;
        totalUpdates = totalInserts + totalRemoves;

        totalSuccessfulGets = GSTATS_GET_STAT_METRICS(num_successful_searches, TOTAL)[0].sum;
        totalSuccessfulInserts = GSTATS_GET_STAT_METRICS(num_successful_inserts, TOTAL)[0].sum;
        totalSuccessfulRemoves = GSTATS_GET_STAT_METRICS(num_successful_removes, TOTAL)[0].sum;
        totalSuccessfulUpdates = totalSuccessfulInserts + totalSuccessfulRemoves;

        totalFailGets = GSTATS_GET_STAT_METRICS(num_fail_searches, TOTAL)[0].sum;
        totalFailInserts = GSTATS_GET_STAT_METRICS(num_fail_inserts, TOTAL)[0].sum;
        totalFailRemoves = GSTATS_GET_STAT_METRICS(num_fail_removes, TOTAL)[0].sum;
        totalFailUpdates = totalFailInserts + totalFailRemoves;

        SECONDS_TO_RUN = _SECONDS_TO_RUN; // (MILLIS_TO_RUN)/1000.;
        totalAll = totalUpdates + totalQueries;
        throughputSearches = (long long) (totalGets / SECONDS_TO_RUN);
        throughputRQs = (long long) (totalRQs / SECONDS_TO_RUN);
        throughputQueries = (long long) (totalQueries / SECONDS_TO_RUN);
        throughputUpdates = (long long) (totalUpdates / SECONDS_TO_RUN);
        throughputAll = (long long) (totalAll / SECONDS_TO_RUN);
    }

    void printTotalStatisticShort(bool detail = false) const {
        COUTATOMIC(std::endl);
        COUTATOMIC("total_gets=" << totalGets << std::endl)
        if (detail) {
            COUTATOMIC("total_successful_gets=" << totalSuccessfulGets << std::endl)
            COUTATOMIC("total_fail_gets=" << totalFailGets << std::endl)
        }
        COUTATOMIC("total_rq=" << totalRQs << std::endl)
        COUTATOMIC("total_inserts=" << totalInserts << std::endl)
        if (detail) {
            COUTATOMIC("total_successful_inserts=" << totalSuccessfulInserts << std::endl)
            COUTATOMIC("total_fail_inserts=" << totalFailInserts << std::endl)
        }
        COUTATOMIC("total_removes=" << totalRemoves << std::endl)
        if (detail) {
            COUTATOMIC("total_successful_removes=" << totalSuccessfulRemoves << std::endl)
            COUTATOMIC("total_fail_removes=" << totalFailRemoves << std::endl)
        }
        COUTATOMIC("total_updates=" << totalUpdates << std::endl)
        if (detail) {
            COUTATOMIC("total_successful_updates=" << totalSuccessfulUpdates << std::endl)
            COUTATOMIC("total_fail_updates=" << totalFailUpdates << std::endl)
        }
        COUTATOMIC("total_queries=" << totalQueries << std::endl)
        COUTATOMIC("total_ops=" << totalAll << std::endl)
        COUTATOMIC("find_throughput=" << throughputSearches << std::endl)
        COUTATOMIC("rq_throughput=" << throughputRQs << std::endl)
        COUTATOMIC("update_throughput=" << throughputUpdates << std::endl)
        COUTATOMIC("query_throughput=" << throughputQueries << std::endl)
        COUTATOMIC("total_throughput=" << throughputAll << std::endl)
        COUTATOMIC(std::endl);
    }

    void printTotalStatistic(bool detail = false) const {
        COUTATOMIC(std::endl)
        COUTATOMIC(indented_title_with_data("total gets", totalGets, 1, 32))
        if (detail) {
            COUTATOMIC(indented_title_with_data("total successful gets", totalSuccessfulGets, 2, 32))
            COUTATOMIC(indented_title_with_data("total fail gets", totalFailGets, 2, 32))
        }
        COUTATOMIC(indented_title_with_data("total rq", totalRQs, 1, 32))
        COUTATOMIC(indented_title_with_data("total inserts", totalInserts, 1, 32))
        if (detail) {
            COUTATOMIC(indented_title_with_data("total successful inserts", totalSuccessfulInserts, 2, 32))
            COUTATOMIC(indented_title_with_data("total fail inserts", totalFailInserts, 2, 32))
        }
        COUTATOMIC(indented_title_with_data("total removes", totalRemoves, 1, 32))
        if (detail) {
            COUTATOMIC(indented_title_with_data("total successful removes", totalSuccessfulRemoves, 2, 32))
            COUTATOMIC(indented_title_with_data("total fail removes", totalFailRemoves, 2, 32))
        }
        COUTATOMIC(indented_title_with_data("total updates", totalUpdates, 1, 32))
        if (detail) {
            COUTATOMIC(indented_title_with_data("total successful updates", totalSuccessfulUpdates, 2, 32))
            COUTATOMIC(indented_title_with_data("total fail updates", totalFailUpdates, 2, 32))
        }
        COUTATOMIC(indented_title_with_data("total queries", totalQueries, 1, 32))
        COUTATOMIC(indented_title_with_data("total ops", totalAll, 1, 32))
        COUTATOMIC(indented_title_with_data("find throughput", throughputSearches, 1, 32))
        COUTATOMIC(indented_title_with_data("rq throughput", throughputRQs, 1, 32))
        COUTATOMIC(indented_title_with_data("update throughput", throughputUpdates, 1, 32))
        COUTATOMIC(indented_title_with_data("query throughput", throughputQueries, 1, 32))
        COUTATOMIC(indented_title_with_data("total throughput", throughputAll, 1, 32))
        COUTATOMIC(std::endl)
    }

};

void to_json(nlohmann::json &json, const Statistic &s) {
    json["total_gets"] = s.totalGets;
    json["total_successful_gets"] = s.totalSuccessfulGets;
    json["total_fail_gets"] = s.totalFailGets;

    json["total_rq"] = s.totalRQs;

    json["total_inserts"] = s.totalInserts;
    json["total_successful_inserts"] = s.totalSuccessfulInserts;
    json["total_fail_inserts"] = s.totalFailInserts;

    json["total_removes"] = s.totalRemoves;
    json["total_successful_removes"] = s.totalSuccessfulRemoves;
    json["total_fail_removes"] = s.totalFailRemoves;

    json["total_updates"] = s.totalUpdates;
    json["total_successful_updates"] = s.totalSuccessfulUpdates;
    json["total_fail_updates"] = s.totalFailUpdates;

    json["total_queries"] = s.totalQueries;
    json["total_ops"] = s.totalAll;
    json["find_throughput"] = s.throughputSearches;
    json["rq_throughput"] = s.throughputRQs;
    json["update_throughput"] = s.throughputUpdates;
    json["query_throughput"] = s.throughputQueries;
    json["total_throughput"] = s.throughputAll;
}

void from_json(const nlohmann::json &json, Statistic &s) {
    s.totalGets = json["total_gets"];
    s.totalSuccessfulGets = json["total_successful_gets"];
    s.totalFailGets = json["total_fail_gets"];

    s.totalRQs = json["total_rq"];

    s.totalInserts = json["total_inserts"];
    s.totalSuccessfulInserts = json["total_successful_inserts"];
    s.totalFailInserts = json["total_fail_inserts"];

    s.totalRemoves = json["total_deletes"];
    s.totalSuccessfulRemoves = json["total_successful_removes"];
    s.totalFailRemoves = json["total_fail_removes"];

    s.totalUpdates = json["total_updates"];
    s.totalSuccessfulUpdates = json["total_successful_updates"];
    s.totalFailUpdates = json["total_fail_updates"];

    s.totalQueries = json["total_queries"];
    s.totalAll = json["total_ops"];
    s.throughputSearches = json["find_throughput"];
    s.throughputRQs = json["rq_throughput"];
    s.throughputUpdates = json["update_throughput"];
    s.throughputQueries = json["query_throughput"];
    s.throughputAll = json["total_throughput"];
}

struct ThreadStatistic {

};

#endif //SETBENCH_STATISTICS_H
