#include "Debugger.h"

#include <readline/history.h>
#include <readline/readline.h>

#include <cstdio>
#include <memory>

#include "Command.h"

namespace {
std::string readInput() {
    char* line = readline("(remu) ");
    std::string ret;
    if (line && *line) {
        add_history(line);
        ret = line;
        free(line);
    }
    return ret;
}

}  // namespace

namespace remu {

int Debugger::Breakpoint::m_counter{0};
int Debugger::Watchpoint::m_counter{0};

void Debugger::Breakpoint::print() const {
    std::printf("[Breakpoint %d]: vaddr = 0x%08x\n", m_id, m_addr);
}

void Debugger::Watchpoint::print() const {
    std::printf("[Watchpoint %d]:\n", m_id);
    for (int vaddr = m_memSpan.first; vaddr <= m_memSpan.second; vaddr += 4) {
        Word_t data = m_mem.vMemRead<Word_t>(vaddr);
        uint8_t* bytes = (uint8_t*)(&data);
        std::printf("0x%08x: %02x %02x %02x %02x\n", vaddr, *bytes,
                    *(bytes + 1), *(bytes + 2), *(bytes + 3));
    }
}

void Debugger::addWatchPoint(Word_t addr) {
    Watchpoint wp({addr, addr + 3}, m_mem);
    int wpId = wp.getId();
    m_mem.addMemWriteTracer(MemTracer(
        wp.getId(), wp.getMemSpan(),
        [wpId, this](Word_t vaddr, Word_t data, int numOfBytes) {
            std::printf(
                "[Watchpoint %d]: write %d bytes at 0x%08x, data = 0x%08x\n",
                wpId, numOfBytes, vaddr, data);
            m_pause = true;
        }));
    m_watchpoints.insert(wp);
}

void Debugger::removeWatchPoint(int id) {
    for (auto& wp : m_watchpoints) {
        if (wp.getId() == id) {
            m_mem.removeMemWriteTracer(id);
            m_watchpoints.erase(wp);
            break;
        }
    }
}

void Debugger::start() {
    while (!m_quit) {
        bool pause = false;
        Breakpoint bp(0, m_cpu.m_pc, m_mem);
        auto itor = m_breakpoints.find(bp);
        if (itor != m_breakpoints.end()) {
            pause = true;
        }

        int num = 0;
        if (pause) {
            auto cmd = handleInput();
            if (!cmd) {
                continue;
            }
            num = cmd->execute();
        }

        if (num > 0) {
            m_cpu.execute(1);
        }
    }
}

std::unique_ptr<ICommand> Debugger::handleInput() {
    std::unique_ptr<ICommand> cmd;
    while (!cmd) {
        std::string input = readInput();
        if (input.empty()) {
            continue;
        }
        // parse input to command
        cmd = parseCommand(*this, input);
        if (!cmd) {
            std::printf("invalid command: %s\n", input.data());
            continue;
        }
        if (!cmd->parse(input)) {
            cmd.reset();
            continue;
        }
        return cmd;
    }
    return cmd;
}
}  // namespace remu