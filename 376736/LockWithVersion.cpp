//
// Created by grisha on 12.11.23.
//

#include "LockWithVersion.hpp"

LockWithVersion::LockWithVersion() {
    vlock.store(0);
}

bool LockWithVersion::try_lock() {
    int cur_vlock = vlock.load();
    if (cur_vlock & 1) {
        return false;
    }
    return vlock.compare_exchange_strong(cur_vlock, cur_vlock + 1);
}

void LockWithVersion::unlock() {
    int cur_vlock = vlock.load();
    if (cur_vlock & 1) {
        vlock.store(cur_vlock - 1);
    }
}

int LockWithVersion::get_version() {
    int cur_vlock = vlock.load();
    return cur_vlock >> 1;
}

bool LockWithVersion::get_lock() {
    int cur_vlock = vlock.load();
    return (bool) (cur_vlock & 1);
}

void LockWithVersion::set_version(int new_v) {
    vlock.store((new_v << 1) | ((int) get_lock()));
}
