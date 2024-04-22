#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include "ISA.h"
#include "Memory.h"
#include "Processor.h"
#include "Util.h"

namespace remu {
class ICommand;
class Debugger {
public:
    class Breakpoint {
    private:
        const int m_id;
        const Word_t m_addr;
        Memory& m_mem;
        static int m_counter;

    public:
        friend struct Hash;
        struct Hash {
            std::size_t operator()(const Breakpoint& bp) const {
                return std::hash<int>()(bp.m_addr);
            }
        };

    public:
        Breakpoint(int id, Word_t addr, Memory& m)
            : m_id(id), m_addr(addr), m_mem(m) {}
        Breakpoint(Word_t addr, Memory& m) : Breakpoint(m_counter++, addr, m) {}
        ~Breakpoint() = default;

        int getId() const { return m_id; }
        Word_t getAddr() const { return m_addr; }

        bool operator==(const Breakpoint& bp) const {
            return m_addr == bp.m_addr;
        }
        void print() const;
    };

    class Watchpoint {
    private:
        const int m_id;
        const MemSpan m_memSpan;
        Memory& m_mem;
        static int m_counter;

    public:
        friend struct Hash;
        struct Hash {
            std::size_t operator()(const Watchpoint& wp) const {
                return std::hash<int>()(wp.m_memSpan.first) ^
                       std::hash<int>()(wp.m_memSpan.second);
            }
        };

    public:
        Watchpoint(int id, MemSpan span, Memory& m)
            : m_id(id), m_memSpan(span), m_mem(m) {}
        Watchpoint(MemSpan span, Memory& m)
            : Watchpoint(m_counter++, span, m) {}
        ~Watchpoint() {}

        int getId() const { return m_id; }
        MemSpan getMemSpan() const { return m_memSpan; }

        bool operator==(const Watchpoint& wp) const {
            return m_memSpan == wp.m_memSpan;
        }
        void print() const;
    };

private:
    bool m_quit;
    bool m_pause;
    std::unordered_set<Breakpoint, Breakpoint::Hash> m_breakpoints;
    std::unordered_set<Watchpoint, Watchpoint::Hash> m_watchpoints;

    Processor& m_cpu;
    Memory& m_mem;

public:
    Debugger(Processor& cpu, Memory& m)
        : m_quit(false),
          m_pause(false),
          m_breakpoints(16),
          m_watchpoints(16),
          m_cpu(cpu),
          m_mem(m) {}
    ~Debugger() = default;

    Processor& getProcessor() { return m_cpu; }
    Memory& getMemory() { return m_mem; }

    void addBreakPoint(Word_t addr) {
        Assert(addr % 4 == 0);
        m_breakpoints.insert(Breakpoint(addr, m_mem));
    }
    void removeBreakPoint(int id) {
        for (auto& bp : m_breakpoints) {
            if (bp.getId() == id) {
                m_breakpoints.erase(bp);
                break;
            }
        }
    }
    void addWatchPoint(Word_t addr);
    void removeWatchPoint(int id);

    void forEachBreakpoint(std::function<void(const Breakpoint&)> f) {
        for (const auto& bp : m_breakpoints) {
            f(bp);
        }
    }
    void forEachWatchpoint(std::function<void(const Watchpoint&)> f) {
        for (const auto& wp : m_watchpoints) {
            f(wp);
        }
    }

    template <typename T>
    void forEach(std::function<void(const T&)> f) {
        if (std::is_same<T, Breakpoint>::value) {
            for (const auto& bp : m_breakpoints) {
                f(bp);
            }
        } else if (std::is_same<T, Watchpoint>::value) {
            for (const auto& wp : m_watchpoints) {
                f(wp);
            }
        }
    }

    void start();
    void quit() { m_quit = true; }

private:
    std::unique_ptr<ICommand> handleInput();
};
}  // namespace remu