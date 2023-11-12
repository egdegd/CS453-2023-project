//
// Created by grisha on 12.11.23.
//

#include "TransactionalMemory.h"
#include "MemorySegment.h"

TransactionalMemory::TransactionalMemory(size_t size, size_t alignment) {
    this->size = size;
    this->alignment = alignment;
    this->memory_segment = new MemorySegment(size, alignment);
    global_clock.store(0);

}

TransactionalMemory::~TransactionalMemory() {

}
