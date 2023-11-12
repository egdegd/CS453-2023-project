//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_LOCKWITHVERSION_H
#define CS453_2023_LOCKWITHVERSION_H


#include <stdatomic.h>
#include <atomic>

class LockWithVersion {
    std::atomic_int lock{};
public:
    LockWithVersion();
};


#endif //CS453_2023_LOCKWITHVERSION_H
