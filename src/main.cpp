#include <iostream>

#include "Memory.h"
#include "Processor.h"
#include "debug/Debugger.h"

namespace remu {
class Machine {
private:
    Memory m_mem;
    Processor m_cpu;
    Debugger m_debugger;

public:
    Machine() : m_mem(), m_cpu(m_mem), m_debugger(m_cpu, m_mem) {}
    ~Machine() {}

    Memory& getMemory() { return m_mem; }
    Processor& getProcessor() { return m_cpu; }
    Debugger& getDebugger() { return m_debugger; }

    void start() {
        try {
            m_cpu.execute(UINT64_MAX);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    void debug() { m_debugger.start(); }

    void stop() {}
};
}  // namespace remu

static const Word_t img[] = {
    0x00000297,  // auipc t0,0
    0x00028823,  // sb  zero,16(t0)
    0x0102c503,  // lbu a0,16(t0)
    0x00100073,  // ebreak (used as nemu_trap)
    0xdeadbeef,  // some data
};

static void copySampleCode(remu::Machine& m) {
    Word_t addr = remu::MemBase;
    for (Word_t inst : img) {
        m.getMemory().vMemWrite<Word_t>(addr, inst);
        addr += 4;
    }
}

int main() {
    remu::Machine machine;
    copySampleCode(machine);

    machine.getDebugger().addBreakPoint(remu::MemBase);

    machine.debug();

    return 0;
}