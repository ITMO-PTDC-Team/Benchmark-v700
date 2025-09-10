//
// Created by Ravil Galiev on 21.07.2023.
//

#ifndef SETBENCH_ARGS_GENERATOR_H
#define SETBENCH_ARGS_GENERATOR_H

#include <utility>
#include <vector>

struct ArgsGenerator {
    virtual size_t nextGet() = 0;

    virtual size_t nextInsert() = 0;

    virtual size_t nextRemove() = 0;

    virtual std::pair<size_t, size_t> nextRange() = 0;

    virtual ~ArgsGenerator() = default;

    /*
        Function, that returns a vector of DataMap ptr's, each corresponding to
        function in a data structure
        1. get
        2. insert
        3. remove
        4. rangeQuery
    */
    virtual std::vector<shared_ptr<DataMap>> getInternalDataMaps() = 0;
};

#endif //SETBENCH_ARGS_GENERATOR_H
