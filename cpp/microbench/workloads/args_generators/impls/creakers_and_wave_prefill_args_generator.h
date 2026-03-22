#pragma once

#include "args_generators/args_generator.h"
#include "workloads/args_generators/impls/creakers_and_wave_args_generator.h"
#include "errors.h"

namespace microbench::workload {

using KeyType = int64_t;

class CreakersAndWavePrefillArgsGenerator : public ArgsGenerator {
    PAD;
    Random64& rng_;
    PAD;
    DataMapPtr data_map_;
    size_t wave_begin_;
    size_t prefill_length_;
    PAD;

public:
    CreakersAndWavePrefillArgsGenerator(Random64& rng, size_t wave_begin, size_t prefill_length,
                                        DataMapPtr data_map)
        : rng_(rng),
          wave_begin_(wave_begin),
          prefill_length_(prefill_length),
          data_map_(std::move(data_map)) {
    }

    KeyType next_get() override{setbench_error("Unsupported operation -- nextGet")}

    KeyType next_insert() override {
        /**
         *                       waveBegin           creakersBegin
         * |_________________________|....................|,,,,,,,,,,,,,,,,,,,,|
         *                           |<--          prefillLength            -->|
         * |....| --- wave
         * |,,,,| --- creakers
         * |____| --- unused data
         */
        return data_map_->get(wave_begin_ + rng_.next(prefill_length_));
    }

    KeyType next_remove() override{setbench_error("Unsupported operation -- nextRemove")}

    std::pair<KeyType, KeyType> next_range() override {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~CreakersAndWavePrefillArgsGenerator() override = default;
};

}  // namespace microbench::workload
