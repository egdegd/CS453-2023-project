//
// Created by grisha on 19.11.23.
//

#include <iostream>
#include "Transaction.hpp"

Transaction::Transaction(TransactionalMemory* tm, bool is_ro) {
    this->tm = tm;
    this->is_ro = is_ro;
    alignment = tm->alignment;
    read_v = tm->global_clock.load();
    write_v = -1;
}

void Transaction::write_to_local(void const* source, std::size_t size, void *target, uint16_t segment_id) {
    for(size_t i = 0; i < size / alignment; i++) {
        void* new_data = malloc(alignment);
        memcpy(new_data, (char*)source + i * alignment, alignment);
        local_write_data.push_back(WriteData{new_data, (char*)target + i * alignment, segment_id});
    }
}

bool Transaction::try_lock_write() {
    for (WriteData data: local_write_data) {
        LockWithVersion* versioned_lock = tm->memory_segments[data.segment_id]->get_vlock(data.destination);
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
        LockWithVersion* versioned_lock = tm->memory_segments[data.segment_id]->get_vlock(data.destination);
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
}

bool Transaction::validate_read() {
    if (read_v + 1 == write_v) {
        return true;
    }
    for (ReadData rd: local_read_data) {
        LockWithVersion* versioned_lock = tm->memory_segments[rd.segment_id]->get_vlock(rd.data);
//        TODO: подумать, все ли правильно, так как версия и лок могут быть полуены в разные моменты
        int version = versioned_lock->get_version();
        bool lock = versioned_lock->get_lock();
        if (version > read_v) return false;
        if (locked_segments.find(versioned_lock) == locked_segments.end()) {
            if (lock) return false;
        }
    }
    return true;
}

void Transaction::make_write() {
    for (WriteData wd: local_write_data) {
        LockWithVersion* versioned_lock = tm->memory_segments[wd.segment_id]->get_vlock(wd.destination);
        memcpy(wd.destination, wd.data, alignment);
        versioned_lock->set_version(write_v);
    }
    unlock_all_write();
}

bool Transaction::rw_read(void *source, std::size_t size, void *target, uint16_t segment_id) {
    for(size_t i = 0; i < size / alignment; i++) {
        void* real_source = (char*) source + i * alignment;
        LockWithVersion* versioned_lock = tm->memory_segments[segment_id]->get_vlock(real_source);
//        TODO: не рано ли делается проверка?
        if (versioned_lock->get_lock() || versioned_lock->get_version() > read_v) return false;
        local_read_data.push_back(ReadData{real_source, segment_id});
        void* real_target = (char*) target + i * alignment;
        size_t j = 0;
        while (j < local_write_data.size()) {
            if (local_write_data[j].destination == real_source) {
                memcpy(real_target, local_write_data[j].data, alignment);
                break;
            }
            j++;
        }
        if (j >= local_write_data.size()) {
            memcpy(real_target, real_source, alignment);
        }
    }
    return true;
}

bool Transaction::ro_read(void *source, std::size_t size, void *target, uint16_t segment_id) {
    for(size_t i = 0; i < size / alignment; i++) {
        void* real_source = (char*) source + i * alignment;
        LockWithVersion* versioned_lock = tm->memory_segments[segment_id]->get_vlock(real_source);
        void* real_target = (char*) target + i * alignment;
        memcpy(real_target, real_source, alignment);
        if (versioned_lock->get_lock() || versioned_lock->get_version() > read_v) return false;
//        TODO: надо ли добавлять в read_set?
    }
    return true;
}

Transaction::~Transaction() {
    for (WriteData w: local_write_data) {
        free(w.data);
    }
    local_read_data.clear();
//    TODO: тут точно так?
    local_write_data.clear();
    tm->lock_free.unlock_shared();
}

bool Transaction::read(void *source, std::size_t size, void *target, uint16_t segment_id) {
    if (is_ro) {
        return ro_read(source, size, target, segment_id);
    } else {
        return rw_read(source, size, target, segment_id);
    }
}
