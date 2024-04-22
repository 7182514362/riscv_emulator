#pragma once

#include "ISA.h"

namespace remu {
enum class ExceptionCause : uint32_t {
    InstAddrMisAligned = 0,
    InstAccessFault,
    IllegalInst,
    Breakpoint,
    LoadAddrMisAligned,
    LoadAccessFault,
    StoreAmoAddrMisAligned,
    StoreAmoAccessFault,
    ECallFromUMode,
    ECallFromSMode,
    // 10 reserved
    ECallFromMMode = 11,
    InstPageFault,
    LoadPageFault,
    // 14 reserved
    StoreAmoPageFault = 15,

    // interrupts
    SSoftInt = (1U << 31) + 1,
    MSoftInt = (1U << 31) + 3,
    STimerInt = (1U << 31) + 5,
    MTimerInt = (1U << 31) + 7,
    SExtInt = (1U << 31) + 9,
    MExtInt = (1U << 31) + 11
};
}