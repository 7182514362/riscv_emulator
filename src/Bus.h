#pragma once

#include "Processor.h"
#include "Memory.h"

namespace remu {
struct Bus {
    Processor& cpu; 
};
}