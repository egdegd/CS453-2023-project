//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_TRANSACTIONALMEMORY_H
#define CS453_2023_TRANSACTIONALMEMORY_H

#include <exception>
#include "atomic"
#include "MemorySegment.h"
#include <mutex>
#include <shared_mutex>
#include <vector>

class TransactionalMemory {
    size_t size;
    size_t alignment;
    std::atomic_int global_clock;
    MemorySegment* memory_segment;
    TransactionalMemory(size_t size, size_t alignment);
    ~TransactionalMemory();
};


#endif //CS453_2023_TRANSACTIONALMEMORY_H
