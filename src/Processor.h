#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "ISA.h"
#include "Instruction.h"
#include "Memory.h"

namespace remu {
constexpr int RegNum = 32;
constexpr int RegWidth = 32;
constexpr int XLEN = RegWidth;

struct Registers {
    std::array<Word_t, RegNum> x;  // general Registers

    // Contral Status Registers
    // Machine Mode
    Word_t mstatus;   // Machine Status Register
    Word_t mip;       // Machine Interrupt Pending
    Word_t mie;       // Machine Interrupt Enable
    Word_t mcause;    // Machine Cause Register
    Word_t mtvec;     // Machine Trap-Vector Base-Address Register
    Word_t mtval;     // Machine Trap Value Register
    Word_t mepc;      // Machine Exception Program Counter
    Word_t mscratch;  // Machine Scratch
};

enum class ProcessorMode { U_MODE, S_MODE, M_MODE };

class Processor {
private:
    Word_t m_pc;   // current pc
    Word_t m_npc;  // next pc
    Registers m_regs;

    Memory& m_mem;

    friend class Debugger;

public:
    Processor(Memory& m) : m_pc(MemBase), m_npc(m_pc), m_mem(m) {
        Instruction::init();
    }
    ~Processor() = default;

    Word_t& pc() { return m_pc; }

    Word_t& npc() { return m_npc; }

    Word_t& reg(uint32_t i) { return m_regs.x[i]; }
    Word_t getGeneralRegFromName(const std::string_view name);

    Memory& getMemory() { return m_mem; }

    void printGeneralReg() const;

    void execute(uint64_t n);

private:
    void init();
    Instruction fetchInst();
};
}  // namespace remu