//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_TRANSACTIONALMEMORY_HPP
#define CS453_2023_TRANSACTIONALMEMORY_HPP

//#include <exception>
#include "atomic"
#include "MemorySegment.hpp"
#include <shared_mutex>
//#include <mutex>
#include <vector>

class TransactionalMemory {


    ~TransactionalMemory();

public:
    TransactionalMemory(size_t size, size_t alignment);

    size_t size;
    size_t alignment;
    size_t max_n_of_segments = 1000;
    std::atomic_uint16_t real_n_of_segments{};
    std::atomic_int global_clock{0};
//    MemorySegment* first_memory_segment;
    MemorySegment** memory_segments;
    std::shared_mutex lock_free;
    std::shared_mutex lock_alloc;
    std::size_t* segment_states;
    const int clean_time = 100000;
//    TODO: rename std::atomic_int transactions_committed_since_last_free{0};
    std::atomic_int transactions_committed_since_last_free{0};
//    TODO: rename void fee_useless_segments();
    void fee_useless_segments();
    static void *create_opaque_data_pointer(void *p, uint16_t segment_id);

    static void *change_pointer_top_digits_to(void const *pVoid, uint16_t id);

    static uint16_t get_pointer_top_digits(void const *pSegment);
    uint16_t real_segment_address_prefix;

    void *real_data_pointer(const void *pVoid) const;

    void add_segments(uint16_t i);
};


#endif //CS453_2023_TRANSACTIONALMEMORY_HPP
