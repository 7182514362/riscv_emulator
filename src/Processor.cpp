
#include "Processor.h"

#include "Util.h"

namespace {
std::array<std::string_view, remu::RegNum> g_regName{
    "$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

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
}  // namespace

namespace remu {
Word_t Processor::getGeneralRegFromName(const std::string_view name) {
    for (int i = 0; i < g_regName.size(); ++i) {
        if (g_regName[i] == name) {
            return m_regs.x[i];
        }
    }
    ThrowRuntimeError("unkown reg name: " + std::string(name));
    return 0;
}

Instruction Processor::fetchInst() {
    Word_t bits = m_mem.vMemRead<Word_t>(m_pc);
    m_npc = m_pc + 4;
    Instruction inst(bits);
    return inst;
}

void Processor::execute(uint64_t n) {
    while ((n--) > 0) [[likely]] {
        // fetch
        Instruction inst = fetchInst();
        // decode
        auto op = inst.decode();
        // execute
        op(*this, m_mem, inst.getBits());
        m_pc = m_npc;
    }
}

void Processor::printGeneralReg() const {
    for (int i = 0; i < g_regName.size(); i += 4) {
        for (int j = 0; j < 4; ++j) {
            std::printf("%s = 0x%08x,\t", g_regName[i + j].data(),
                        m_regs.x[i + j]);
        }
        std::printf("\n");
    }
}
}  // namespace remu