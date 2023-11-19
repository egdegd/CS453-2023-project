//
// Created by grisha on 12.11.23.
//

#include "TransactionalMemory.h"
#include "MemorySegment.h"


TransactionalMemory::TransactionalMemory(size_t size, size_t alignment) {
    this->size = size;
    this->alignment = alignment;
    // TODO: delete first memory segment. Or not?
//    this->first_memory_segment = new MemorySegment(size, alignment);
    this->memory_segments = new MemorySegment*[N_OF_SEGMENTS];
    for (size_t i = 0; i < N_OF_SEGMENTS; i++) {
        memory_segments[i] = nullptr;
    }
//    memory_segments[0] = first_memory_segment;
    global_clock.store(0);

}

TransactionalMemory::~TransactionalMemory() {

}
