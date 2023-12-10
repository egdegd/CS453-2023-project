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
    size_t max_n_of_segments = 6000;
    std::atomic_uint16_t real_n_of_segments{};
    std::atomic_int global_clock{0};
    MemorySegment** memory_segments;
    std::shared_mutex lock_free;
    std::shared_mutex lock_alloc;
    std::size_t* segment_states;
    std::atomic_int n_of_commited_trans{0};
    void free_useless_segments();
    static void *create_temp_pointer(void *p, uint16_t segment_id);

    static void *change_first_digits(void const *pVoid, uint16_t id);

    static uint16_t get_first_digits(void const *pSegment);
    uint16_t real_addr;

    void *real_data_pointer(const void *pVoid) const;

    void add_segments();
};


#endif //CS453_2023_TRANSACTIONALMEMORY_HPP
