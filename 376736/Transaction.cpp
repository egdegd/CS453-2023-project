//
// Created by grisha on 19.11.23.
//

#include "Transaction.h"

Transaction::Transaction(TransactionalMemory* tm, bool is_ro) {
    this->tm = tm;
    this->is_ro = is_ro;
    alignment = tm->alignment;
    read_v = tm->global_clock.load();
    write_v = -1;
}

void Transaction::write_to_local(const void *source, std::size_t size, void *target, uint16_t segment_id) {
    for(size_t i = 0; i < size / alignment; i++) {
        void* new_data = malloc(alignment);
        memcpy(new_data, (char*)source + i * alignment, alignment);
        local_write_data.push_back(WriteData{(char*)target + i * alignment, new_data, segment_id});
    }
}
bool Transaction::try_lock_write() {
    for (WriteData data: local_write_data) {
        LockWithVersion* versioned_lock = tm->memory_segments[data.segment_id]->get_lock(data.data);
//        TODO: Is it necessary?
        if (locked_segments.find(versioned_lock) != locked_segments.end()) {
            continue;
        }
        if(!versioned_lock->try_lock()) {
            unlock_all_write();
            return false;
        }
        locked_segments.insert(versioned_lock);
    }
    return true;
}
void Transaction::unlock_all_write() {
    for (WriteData data: local_write_data) {
        LockWithVersion* versioned_lock = tm->memory_segments[data.segment_id]->get_lock(data.data);
        if (locked_segments.find(versioned_lock) != locked_segments.end()) {
            versioned_lock->unlock();
            locked_segments.erase(versioned_lock);
        }
    }
}

bool Transaction::end() {
    if (is_ro) {
//        TODO: why?
        return true;
    } else {
        if (!try_lock_write()) {
            return false;
        }
        write_v = atomic_fetch_add(&tm->global_clock, 1) + 1;
        if (!validate_read()) {
            unlock_all_write();
            return false;
        }
        make_write();
        return true;
    }
    return false;
}
