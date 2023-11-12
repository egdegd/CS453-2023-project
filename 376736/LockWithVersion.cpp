//
// Created by grisha on 12.11.23.
//

#include "LockWithVersion.h"

LockWithVersion::LockWithVersion() {
    lock.store(0 );
}
