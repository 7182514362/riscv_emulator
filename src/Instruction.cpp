#include "Instruction.h"

#include <cstdint>
#include <unordered_map>

#include "Memory.h"
#include "Processor.h"
#include "Util.h"

namespace {

template <typename T, unsigned Width>
inline T signExtend(const T x) {
    struct {
        T n : Width;
    } s = {.n = x};
    return s.n;
}

inline Word_t opcode(const Word_t inst) { return inst & 0x7F; }

inline Word_t rd(const Word_t inst) { return (inst >> 7) & 0x1F; }

inline Word_t rs1(const Word_t inst) { return (inst >> 15) & 0x1F; }

inline Word_t rs2(const Word_t inst) { return (inst >> 20) & 0x1F; }

inline Word_t funct3(const Word_t inst) { return (inst >> 12) & 0x7; }

inline Word_t funct7(const Word_t inst) { return (inst >> 25) & 0x7F; }

inline Word_t shamt(const Word_t inst) { return (inst >> 20) & 0x3F; }

constexpr Word_t mask(const uint32_t h, const uint32_t l) {
    return (1ull << (h - l + 1)) - 1;
}

template <const int H, const int L>
inline Word_t imm(const Word_t inst) {
    return (inst >> L) & mask(H, L);
}

inline Word_t immI(const Word_t inst) { return imm<31, 20>(inst); }

inline Word_t immS(const Word_t inst) {
    return (imm<31, 25>(inst) << 5) | imm<11, 7>(inst);
}

inline Word_t immB(const Word_t inst) {
    return (imm<31, 31>(inst) << 12) | (imm<30, 25>(inst) << 5) |
           (imm<11, 8>(inst) << 1) | (imm<7, 7>(inst) << 11);
}

inline Word_t immU(const Word_t inst) { return imm<31, 12>(inst); }

inline Word_t immJ(const Word_t inst) {
    return (imm<31, 31>(inst) << 20) | (imm<30, 21>(inst) << 1) |
           (imm<20, 20>(inst) << 11) | (imm<19, 12>(inst) << 12);
}

inline Word_t pred(const Word_t inst) { return imm<27, 24>(inst); }

inline Word_t succ(const Word_t inst) { return imm<23, 20>(inst); }

constexpr uint32_t opcode_mask(uint32_t bits) { return bits & 0x7F; }
constexpr uint32_t funct3_mask(uint32_t bits) { return (bits & 0x7) << 12; }
constexpr uint32_t funct7_mask(uint32_t bits) { return (bits & 0x7F) << 25; }

}  // namespace

namespace remu {

std::unordered_map<uint32_t, Operation> Instruction::m_instMap;
std::vector<InstructionDecodeInfo> Instruction::m_instList{
    // R type
    {"add|sub|sll|slt|sltu|xor|srl|sra|or|and", opcode_mask(0b0110011),
     InstructionFormat::IF_R,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         uint32_t funct = (funct7(inst) << 3) | funct3(inst);
         switch (funct) {
             case 0b000'0000'000:  // add
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) + cpu.reg(rs2(inst));
                 break;
             case 0b010'0000'000:  // sub
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) - cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'001:  // sll
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) << cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'010:  // slt
                 cpu.reg(rd(inst)) =
                     (int32_t)cpu.reg(rs1(inst)) < (int32_t)cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'011:  // sltu
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) < cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'100:  // xor
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) ^ cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'101:  // srl
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) >> cpu.reg(rs2(inst));
                 break;
             case 0b010'0000'101:  // sra
                 cpu.reg(rd(inst)) =
                     (int32_t)cpu.reg(rs1(inst)) >> cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'110:  // or
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) | cpu.reg(rs2(inst));
                 break;
             case 0b000'0000'111:  // and
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) & cpu.reg(rs2(inst));
                 break;

             case 0b000'0001'000:  // mul
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) * cpu.reg(rs2(inst));
                 break;
             case 0b000'0001'001:  // mulh
                 cpu.reg(rd(inst)) = ((int64_t)cpu.reg(rs1(inst)) *
                                      (int64_t)cpu.reg(rs2(inst))) >>
                                     XLEN;
                 break;
             case 0b000'0001'010:  // mulhsu
                 cpu.reg(rd(inst)) = ((int64_t)cpu.reg(rs1(inst)) *
                                      (uint64_t)cpu.reg(rs2(inst))) >>
                                     XLEN;
                 break;
             case 0b000'0001'011:  // mulhu
                 cpu.reg(rd(inst)) = ((uint64_t)cpu.reg(rs1(inst)) *
                                      (uint64_t)cpu.reg(rs2(inst))) >>
                                     XLEN;
                 break;
             case 0b000'0001'100:  // div
                 cpu.reg(rd(inst)) =
                     (int32_t)cpu.reg(rs1(inst)) / (int32_t)cpu.reg(rs2(inst));
                 break;
             case 0b000'0001'101:  // divu
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) / cpu.reg(rs2(inst));
                 break;
             case 0b000'0001'110:  // rem
                 cpu.reg(rd(inst)) =
                     (int32_t)cpu.reg(rs1(inst)) % (int32_t)cpu.reg(rs2(inst));
                 break;
             case 0b000'0001'111:  // remu
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) % cpu.reg(rs2(inst));
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"lb|lh|lw|lbu|lhu", opcode_mask(0b000'0011), InstructionFormat::IF_I,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         Word_t addr = cpu.reg(rs1(inst)) + signExtend<int32_t, 12>(immI(inst));
         switch (funct3(inst)) {
             case 0b000:  // lb
                 cpu.reg(rd(inst)) = signExtend<int32_t, 8>(
                     mem.vMemReadWithTrace<uint8_t>(addr));
                 break;
             case 0b100:  // lbu
                 cpu.reg(rd(inst)) = mem.vMemReadWithTrace<uint8_t>(addr);
                 break;
             case 0b001:  // lh
                 cpu.reg(rd(inst)) = signExtend<int32_t, 16>(
                     mem.vMemReadWithTrace<uint16_t>(addr));
                 break;
             case 0b101:  // lhu
                 cpu.reg(rd(inst)) = mem.vMemReadWithTrace<uint16_t>(addr);
                 break;
             case 0b010:  // lw
                 cpu.reg(rd(inst)) = signExtend<int32_t, 32>(
                     mem.vMemReadWithTrace<uint32_t>(addr));
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"sb|sh|sw", opcode_mask(0b010'0011), InstructionFormat::IF_S,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         Word_t addr = cpu.reg(rs1(inst)) + signExtend<int32_t, 12>(immS(inst));
         switch (funct3(inst)) {
             case 0b000:  // sb
                 mem.vMemWriteWithTrace<uint8_t>(addr, cpu.reg(rs2(inst)));
                 break;
             case 0b001:  // sh
                 mem.vMemWriteWithTrace<uint16_t>(addr, cpu.reg(rs2(inst)));
                 break;
             case 0b010:  // sw
                 mem.vMemWriteWithTrace<uint32_t>(addr, cpu.reg(rs2(inst)));
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"addi|slti|sltiu|xori|ori|andi|slli|srli|srai", opcode_mask(0b001'0011),
     InstructionFormat::IF_I,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         switch (funct3(inst)) {
             case 0b000:  // addi:         000
                 cpu.reg(rd(inst)) =
                     cpu.reg(rs1(inst)) + signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b010:  // slti:         010
                 cpu.reg(rd(inst)) =
                     (int32_t)cpu.reg(rs1(inst)) <
                     (int32_t)signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b011:  // sltiu:        011
                 cpu.reg(rd(inst)) =
                     cpu.reg(rs1(inst)) < signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b100:  // xori:         100
                 cpu.reg(rd(inst)) =
                     cpu.reg(rs1(inst)) ^ signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b110:  // ori:          110
                 cpu.reg(rd(inst)) =
                     cpu.reg(rs1(inst)) | signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b111:  // andi:         111
                 cpu.reg(rd(inst)) =
                     cpu.reg(rs1(inst)) & signExtend<int32_t, 12>(immI(inst));
                 break;
             case 0b001:  // slli: 00'0000  001
                 cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) << shamt(inst);
             case 0b101:
                 // srli: 000'0000  101 shamt=rs2
                 if (funct7(inst) == 0) {
                     cpu.reg(rd(inst)) = cpu.reg(rs1(inst)) >> shamt(inst);
                 } else if (funct7(inst) == 0b010'0000) {
                     // srai: 010'0000  101
                     cpu.reg(rd(inst)) =
                         (int32_t)cpu.reg(rs1(inst)) >> shamt(inst);
                 } else {
                     InvalidInstruction(inst, cpu.pc());
                 }
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"fence|fence.i", opcode_mask(0b000'1111), InstructionFormat::IF_I,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         ThrowRuntimeError("unimplemented instruction");
     }},
    {"ecall|ebreak|csrrw|csrrs|csrrc|csrrwi|csrrsi|csrrci",
     opcode_mask(0b111'0011), InstructionFormat::IF_I,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         switch (funct3(inst)) {
             case 0b000:  // ecall ebreak
                 switch (immI(inst)) {
                     case 0:  // ecall
                         break;
                     case 1:  // ebreak
                         break;
                     default:
                         InvalidInstruction(inst, cpu.pc());
                         break;
                 }
                 break;
             case 0b001:  // csrrw
                          //  auto csr = immI(inst);
                 break;
             case 0b010:  // csrrs
                 break;
             case 0b011:  // csrrc
                 break;
             case 0b101:  // csrrwi
                 break;
             case 0b110:  // csrrsi
                 break;
             case 0b111:  // csrrci
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"beq|bne|blt|bge|bltu|bgeu", opcode_mask(0b110'0011),
     InstructionFormat::IF_B,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         switch (funct3(inst)) {
             case 0b000:  // beq
                 if (cpu.reg(rs1(inst)) == cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             case 0b101:  // bge
                 if ((int32_t)cpu.reg(rs1(inst)) >=
                     (int32_t)cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             case 0b111:  // bgeu
                 if (cpu.reg(rs1(inst)) >= cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             case 0b100:  // blt
                 if ((int32_t)cpu.reg(rs1(inst)) <
                     (int32_t)cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             case 0b110:  // bltu
                 if (cpu.reg(rs1(inst)) < cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             case 0b001:  // bne
                 if (cpu.reg(rs1(inst)) != cpu.reg(rs2(inst))) {
                     cpu.npc() = cpu.pc() + signExtend<int32_t, 13>(immB(inst));
                 }
                 break;
             default:
                 InvalidInstruction(inst, cpu.pc());
                 break;
         }
     }},
    {"lui", opcode_mask(0b011'0111), InstructionFormat::IF_U,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         cpu.reg(rd(inst)) = signExtend<int32_t, 32>(immU(inst) << 12);
     }},
    {"auipc", opcode_mask(0b001'0111), InstructionFormat::IF_U,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         cpu.reg(rd(inst)) =
             cpu.pc() + signExtend<int32_t, 32>(immU(inst) << 12);
     }},
    {"jal", opcode_mask(0b110'1111), InstructionFormat::IF_J,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         cpu.reg(rd(inst)) = cpu.pc() + 4;
         cpu.npc() = cpu.pc() + signExtend<int32_t, 20>(immJ(inst));
     }},
    {"jalr", opcode_mask(0b110'0111), InstructionFormat::IF_I,
     [](Processor& cpu, Memory& mem, uint32_t inst) {
         Word_t t = cpu.pc() + 4;
         cpu.npc() =
             (cpu.reg(rs1(inst)) + signExtend<int32_t, 12>(immI(inst))) & (~1);
         cpu.reg(rd(inst)) = t;
     }}};

void Instruction::init() {
    for (const auto& info : m_instList) {
        m_instMap.emplace(info.opcode, info.op);
    }
}

Operation Instruction::decode() {
    auto itor = m_instMap.find(opcode(m_bits));
    if (itor == m_instMap.end()) [[unlikely]] {
        InvalidInstruction(m_bits, 0);
    }
    return itor->second;
}
}  // namespace remu