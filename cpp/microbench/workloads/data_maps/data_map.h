#ifndef SETBENCH_DATA_MAP_H
#define SETBENCH_DATA_MAP_H

#include "globals_extern.h"

class DataMap {
protected:
    size_t mapId;
    KEY_TYPE* actualData;

public:
    virtual KEY_TYPE* convert(size_t index) = 0;

    virtual void init(size_t range) = 0;

    virtual ~DataMap() = default;

    virtual size_t getId() const { return mapId; }
    
    virtual KEY_TYPE get_min() const = 0;
    virtual KEY_TYPE get_max(size_t range) const = 0;

    virtual std::string toString(size_t indents = 1) = 0;

    virtual void toJson(nlohmann::json &j) const = 0;
};

#endif //SETBENCH_DATA_MAP_H
