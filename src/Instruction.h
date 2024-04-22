#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

#include "ISA.h"

namespace remu {
class Processor;
class Memory;
using Operation = std::function<void(Processor&, Memory&, uint32_t inst)>;

enum class InstructionFormat : uint8_t {
    IF_R = 0,
    IF_I,
    IF_S,
    IF_B,
    IF_U,
    IF_J
};

struct InstructionDecodeInfo {
    std::string_view name;
    uint32_t opcode;
    InstructionFormat type;
    Operation op;
};

class Instruction {
private:
    uint32_t m_bits;

    static std::unordered_map<uint32_t, Operation> m_instMap;
    static std::vector<InstructionDecodeInfo> m_instList;

public:
    Instruction(uint32_t bits) : m_bits(bits) {}
    ~Instruction() {}

    Operation decode();

    uint32_t getBits() const { return m_bits; }

    static void init();

private:
    // Word_t& reg(int i) { return m_cpu.getReg(i); }
};
}  // namespace remu