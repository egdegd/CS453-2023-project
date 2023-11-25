//
// Created by grisha on 12.11.23.
//

#ifndef CS453_2023_LOCKWITHVERSION_H
#define CS453_2023_LOCKWITHVERSION_H


#include <atomic>

class LockWithVersion {
    std::atomic_int vlock{};
public:
    LockWithVersion();
    bool try_lock();
    void unlock();
    int get_version();
    bool get_lock();
    void set_version(int new_v);
};


#endif //CS453_2023_LOCKWITHVERSION_H
