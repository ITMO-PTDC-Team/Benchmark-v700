#pragma once

#include "workloads/args_generators/impls/creakers_and_wave_args_generator.h"
#include "errors.h"

namespace microbench::workload {
template <typename K>
class CreakersAndWavePrefillArgsGenerator : public ArgsGenerator<K> {
    PAD;
    Random64& rng_;
    PAD;
    DataMap<K>* data_map_;
    size_t wave_begin_;
    size_t prefill_length_;
    PAD;

public:
    CreakersAndWavePrefillArgsGenerator(Random64& rng, size_t wave_begin, size_t prefill_length,
                                        DataMap<K>* data_map)
        : rng_(rng),
          wave_begin_(wave_begin),
          prefill_length_(prefill_length),
          data_map_(data_map) {
    }

    K next_get() override{setbench_error("Unsupported operation -- nextGet")}

    K next_insert() override {
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

    K next_remove() override{setbench_error("Unsupported operation -- nextRemove")}

    std::pair<K, K> next_range() override {
        setbench_error("Unsupported operation -- nextRange")
    }

    ~CreakersAndWavePrefillArgsGenerator() override {
        delete data_map_;
    };
};

}  // namespace microbench::workload
