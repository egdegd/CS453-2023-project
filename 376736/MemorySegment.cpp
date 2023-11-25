//
// Created by grisha on 12.11.23.
//

#include <cstdlib>
#include "MemorySegment.h"
#include "cstring"

MemorySegment::MemorySegment(size_t size, size_t alignment) {
    this->size = size;
    this->alignment = alignment;
    this->data = static_cast<char *>(aligned_alloc(alignment, size));
    memset(data, 0, size);
    this->locks = new LockWithVersion[size / alignment];
}

LockWithVersion *MemorySegment::get_vlock(void *p) const {
    return &locks[((char*)p - data) / alignment];
}
