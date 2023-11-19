//
// Created by grisha on 19.11.23.
//

#ifndef CS453_2023_TRANSACTION_H
#define CS453_2023_TRANSACTION_H


#include <set>
#include "TransactionalMemory.h"

struct WriteData {
    void* data;
    void* destination;
    uint16_t segment_id;
};

struct ReadData {
    void* data;
    uint16_t segment_id;
};

class Transaction {
public:
    bool is_ro{};
    size_t alignment;
    TransactionalMemory* tm;
    size_t read_v;
    size_t write_v;
    void write_to_local(void const* source, std::size_t size, void* target, uint16_t segment_index);
    bool end();
    Transaction(TransactionalMemory* tm, bool is_ro);
    std::vector<ReadData> local_read_data{};
    std::vector<WriteData> local_write_data{};
    std::set<LockWithVersion*> locked_segments{};
    bool try_lock_write();

    void unlock_all_write();
};


#endif //CS453_2023_TRANSACTION_H
