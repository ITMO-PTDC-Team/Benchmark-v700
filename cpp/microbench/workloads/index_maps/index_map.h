//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_INDEX_MAP_H
#define SETBENCH_INDEX_MAP_H

class IndexMap {

public:
    virtual size_t get(size_t index) = 0;

    virtual ~IndexMap() = default;
};

#endif //SETBENCH_INDEX_MAP_H
