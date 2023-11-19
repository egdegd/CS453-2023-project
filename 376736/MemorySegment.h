//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_MEMORYSEGMENT_H
#define CS453_2023_MEMORYSEGMENT_H


#include <cstring>
#include "LockWithVersion.h"

class MemorySegment {

public:
    size_t size;
    size_t alignment;
    char* data;
    LockWithVersion* locks;
    LockWithVersion* get_lock(void* p) const;

    MemorySegment(size_t size, size_t alignment);
};


#endif //CS453_2023_MEMORYSEGMENT_H
