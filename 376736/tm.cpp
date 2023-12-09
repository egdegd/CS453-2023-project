/**
 * @file   tm.c
 * @author [...]
 *
 * @section LICENSE
 *
 * [...]
 *
 * @section DESCRIPTION
 *
 * Implementation of your own transaction manager.
 * You can completely rewrite this file (and create more files) as you wish.
 * Only the interface (i.e. exported symbols and semantic) must be preserved.
**/

// Requested features
//#define _GNU_SOURCE
//#define _POSIX_C_SOURCE   200809L
//#ifdef __STDC_NO_ATOMICS__
//    #error Current C11 compiler does not support atomic operations
//#endif

// External headers

// Internal headers
#include <tm.hpp>

#include "macros.h"
#include "TransactionalMemory.hpp"
#include "Transaction.hpp"

/** Create (i.e. allocate + init) a new shared memory region, with one first non-free-able allocated segment of the requested size and alignment.
 * @param size  Size of the first shared segment of memory to allocate (in bytes), must be a positive multiple of the alignment
 * @param align Alignment (in bytes, must be a power of 2) that the shared memory region must support
 * @return Opaque shared memory region handle, 'invalid_shared' on failure
**/
shared_t tm_create(size_t size, size_t align) noexcept {
    try {
        return new TransactionalMemory(size, align);
    } catch (...) {
        return invalid_shared;
    }
}

/** Destroy (i.e. clean-up + free) a given shared memory region.
 * @param shared Shared memory region to destroy, with no running transaction
**/
void tm_destroy(shared_t shared) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    for (size_t i = 0; i < tm->max_n_of_segments; i++) {
        //    TODO: if (tm->segment_states[i] == 2) ... . Maybe add lock_free.try_lock?
        if (tm->segment_states[i] == 1) {
            tm->memory_segments[i]->free();
        }
    }
    delete[] tm->memory_segments;
    delete[] tm->segment_states;
}

/** [thread-safe] Return the start address of the first allocated segment in the shared memory region.
 * @param shared Shared memory region to query
 * @return Start address of the first allocated segment
**/
void* tm_start(shared_t shared) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    return TransactionalMemory::create_opaque_data_pointer(tm->memory_segments[0]->data, 0);
}

/** [thread-safe] Return the size (in bytes) of the first allocated segment of the shared memory region.
 * @param shared Shared memory region to query
 * @return First allocated segment size
**/
size_t tm_size(shared_t shared) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    return tm->size;
}

/** [thread-safe] Return the alignment (in bytes) of the memory accesses on the given shared memory region.
 * @param shared Shared memory region to query
 * @return Alignment used globally
**/
size_t tm_align(shared_t shared) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    return tm->alignment;
}

/** [thread-safe] Begin a new transaction on the given shared memory region.
 * @param shared Shared memory region to start a transaction on
 * @param is_ro  Whether the transaction is read-only
 * @return Opaque transaction ID, 'invalid_tx' on failure
**/
tx_t tm_begin(shared_t shared, bool is_ro) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    tm->fee_useless_segments();
    while(!tm->lock_free.try_lock_shared()) {}
    auto* tr = new Transaction(tm, is_ro);
    return (tx_t) tr;
}

/** [thread-safe] End the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to end
 * @return Whether the whole transaction committed
**/
bool tm_end(shared_t shared, tx_t tx) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    auto* tr = (Transaction*) tx;
    bool is_committed = tr->end();
    delete tr;
    tm->transactions_committed_since_last_free.fetch_add(1);
    return is_committed;
}

/** [thread-safe] Read operation in the given transaction, source in the shared region and target in a private region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in the shared region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in a private region)
 * @return Whether the whole transaction can continue
**/
bool tm_read(shared_t shared, tx_t tx, void const* source, size_t size, void* target) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    auto* tr = (Transaction*) tx;
    uint16_t segment_id = TransactionalMemory::get_pointer_top_digits(source);
    void* p = tm->real_data_pointer(source);
    bool succ = tr->read(p, size, target, segment_id);
    if (!succ) {
        delete tr;
    }
    return succ;
}

/** [thread-safe] Write operation in the given transaction, source in a private region and target in the shared region.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param source Source start address (in a private region)
 * @param size   Length to copy (in bytes), must be a positive multiple of the alignment
 * @param target Target start address (in the shared region)
 * @return Whether the whole transaction can continue
**/
bool tm_write(shared_t shared, tx_t tx, void const* source, size_t size, void* target) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    auto* tr = (Transaction*) tx;
    uint16_t segment_id = TransactionalMemory::get_pointer_top_digits(target);
    void* p = tm->real_data_pointer(target);
    tr->write_to_local(source, size, p, segment_id);
    return true;
}

/** [thread-safe] Memory allocation in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param size   Allocation requested size (in bytes), must be a positive multiple of the alignment
 * @param target Pointer in private memory receiving the address of the first byte of the newly allocated, aligned segment
 * @return Whether the whole transaction can continue (success/nomem), or not (abort_alloc)
**/
Alloc tm_alloc(shared_t shared, tx_t unused(tx), size_t size, void** target) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    uint16_t old = tm->real_n_of_segments.fetch_add(1);
    if (old >= tm->max_n_of_segments) {
        tm->add_segments(old);
    }
    tm->memory_segments[old] = new MemorySegment(size, tm->alignment);
    tm->segment_states[old] = 1;
    *target = TransactionalMemory::create_opaque_data_pointer(tm->memory_segments[old]->data, old);
    return Alloc::success;

}

/** [thread-safe] Memory freeing in the given transaction.
 * @param shared Shared memory region associated with the transaction
 * @param tx     Transaction to use
 * @param target Address of the first byte of the previously allocated segment to deallocate
 * @return Whether the whole transaction can continue
**/
bool tm_free(shared_t shared, tx_t unused(tx), void* target) noexcept {
    auto* tm = (TransactionalMemory*) shared;
    uint16_t segment_id = TransactionalMemory::get_pointer_top_digits(target);
    tm->segment_states[segment_id] = 3;
    return true;
//    while (!tm->lock_free.try_lock()) {}
//    tm->memory_segments[segment_id]->free();
//    delete tm->memory_segments[segment_id];
//    tm->segment_states[segment_id] = 2;
//    tm->lock_free.unlock();
//    return true;
}
