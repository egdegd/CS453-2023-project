//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_TRANSACTIONALMEMORY_H
#define CS453_2023_TRANSACTIONALMEMORY_H

//#include <exception>
#include "atomic"
#include "MemorySegment.h"
#include <shared_mutex>
//#include <mutex>
#include <vector>

class TransactionalMemory {


    ~TransactionalMemory();

public:
    TransactionalMemory(size_t size, size_t alignment);

    size_t size;
    size_t alignment;
    size_t N_OF_SEGMENTS = 10;
    std::atomic_int global_clock{0};
//    MemorySegment* first_memory_segment;
    MemorySegment** memory_segments;
    std::shared_mutex freeing_lock;
};


#endif //CS453_2023_TRANSACTIONALMEMORY_H
