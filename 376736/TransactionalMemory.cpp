//
// Created by grisha on 12.11.23.
//

#include "TransactionalMemory.hpp"
#include "MemorySegment.hpp"


TransactionalMemory::TransactionalMemory(size_t size, size_t alignment) {
    this->size = size;
    this->alignment = alignment;
    // TODO: delete first memory segment. Or not?
//    this->first_memory_segment = new MemorySegment(size, alignment);
    this->memory_segments = new MemorySegment*[max_n_of_segments];
    this->segment_states = new size_t[max_n_of_segments];
    for (size_t i = 0; i < max_n_of_segments; i++) {
        memory_segments[i] = nullptr;
        segment_states[i] = 0;
    }
    memory_segments[0] = new MemorySegment(size, alignment);
    segment_states[0] = 1;
    real_n_of_segments.store(1);
    global_clock.store(0);
//    TODO: rename real_segment_address_prefix;
    real_segment_address_prefix = get_pointer_top_digits(memory_segments[0]);
}

TransactionalMemory::~TransactionalMemory() {

}

void *TransactionalMemory::create_opaque_data_pointer(void *p, uint16_t segment_id) {
    return change_pointer_top_digits_to(p, segment_id);
}

void *TransactionalMemory::change_pointer_top_digits_to(void const *pVoid, uint16_t id) {
//    TODO: rename this function
    unsigned long clear_mask = 0b0000000000000000111111111111111111111111111111111111111111111111;
    return (void*)(((unsigned long) pVoid & clear_mask) | ((unsigned long)id << 48));
}

uint16_t TransactionalMemory::get_pointer_top_digits(void const *pSegment) {
//    TODO: rename
    unsigned long mask = 0b1111111111111111000000000000000000000000000000000000000000000000;
    return ((unsigned long) pSegment & mask) >> 48;
}

void *TransactionalMemory::real_data_pointer(void const *pVoid) const {
    return change_pointer_top_digits_to(pVoid, real_segment_address_prefix);
}

void TransactionalMemory::add_segments(uint16_t i) {
    while(!lock_alloc.try_lock()) {
//      TODO: think about it
        if (i < max_n_of_segments) return;
    }
    max_n_of_segments *= 2;
    memory_segments = static_cast<MemorySegment **>(realloc(memory_segments,
                                                            sizeof(MemorySegment *) * max_n_of_segments));
    segment_states = static_cast<size_t *>(realloc(segment_states, sizeof(size_t *) * max_n_of_segments));
    lock_alloc.unlock();
}

void TransactionalMemory::fee_useless_segments() {
    if (transactions_committed_since_last_free.load() < clean_time) return;

    while (!lock_free.try_lock()){};

    for (std::size_t i = 0; i < max_n_of_segments; i++) {
        if (segment_states[i] == 3) {
            memory_segments[i]->free();
            delete memory_segments[i];
            segment_states[i] = 2;
        }
    }
    transactions_committed_since_last_free.store(0);
    lock_free.unlock();
}
