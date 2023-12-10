//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_MEMORYSEGMENT_HPP
#define CS453_2023_MEMORYSEGMENT_HPP


#include <cstring>
#include "LockWithVersion.hpp"

class MemorySegment {

public:
    size_t size;
    size_t alignment;
    char* data;
    LockWithVersion* locks;
    LockWithVersion* get_vlock(void* p) const;

    MemorySegment(size_t size, size_t alignment);
    ~MemorySegment();
//    void free() const;
};


#endif //CS453_2023_MEMORYSEGMENT_HPP
