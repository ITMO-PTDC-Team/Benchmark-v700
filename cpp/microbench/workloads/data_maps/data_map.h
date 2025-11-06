//
// Created by Ravil Galiev on 24.07.2023.
//

#ifndef SETBENCH_DATA_MAP_H
#define SETBENCH_DATA_MAP_H

template<typename K>
struct DataMap {
    virtual K get(size_t index) = 0;

    virtual ~DataMap() = default;
};

#endif //SETBENCH_DATA_MAP_H
