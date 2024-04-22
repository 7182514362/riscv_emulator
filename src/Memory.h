#pragma once

#include <algorithm>
#include <functional>
#include <list>

#include "ISA.h"
#include "Util.h"

namespace remu {
constexpr Word_t MemBase = 0x80000000;
constexpr Word_t MemSize = 1ull << 27;

using MemSpan = std::pair<Word_t, Word_t>;

class MemTracer {
    using MemTraceFunc =
        std::function<void(Word_t vaddr, Word_t data, int numOfBytes)>;

private:
    int m_id;
    // memory span to be traced
    MemSpan m_span;
    // callback
    MemTraceFunc handler;

public:
    MemTracer(int id, MemSpan span, MemTraceFunc func)
        : m_id(id), m_span(span), handler(func) {}
    ~MemTracer() = default;

    int getId() const { return m_id; }

    bool inSpan(Word_t vaddr) {
        return vaddr >= m_span.first && vaddr <= m_span.second;
    }

    void operator()(Word_t vaddr, Word_t data, int numOfBytes) {
        handler(vaddr, data, numOfBytes);
    }

    bool operator==(const MemTracer *t) const { return m_id == t->m_id; }
};

class Memory {
private:
    uint8_t *m_phyMem;

    std::list<MemTracer> m_memReadTraceList;
    std::list<MemTracer> m_memWriteTraceList;

private:
    void traceMemRead(Word_t vaddr, Word_t data, int numOfBytes);
    void traceMemWrite(Word_t vaddr, Word_t data, int numOfbytes);

public:
    Memory() {
        m_phyMem = static_cast<uint8_t *>(std::aligned_alloc(4096, MemSize));
    }
    ~Memory() {
        if (m_phyMem != nullptr) {
            std::free(m_phyMem);
        }
    }

    void addMemReadTracer(const MemTracer &t) {
        m_memReadTraceList.push_back(t);
    }
    void addMemWriteTracer(const MemTracer &t) {
        m_memWriteTraceList.push_back(t);
    }
    void removeMemReadTracer(int id) {
        auto itor =
            std::find_if(m_memReadTraceList.begin(), m_memReadTraceList.end(),
                         [id](const MemTracer &t) { return t.getId() == id; });
        if (itor != m_memReadTraceList.end()) {
            m_memReadTraceList.erase(itor);
        }
    }
    void removeMemWriteTracer(int id) {
        auto itor =
            std::find_if(m_memWriteTraceList.begin(), m_memWriteTraceList.end(),
                         [id](const MemTracer &t) { return t.getId() == id; });
        if (itor != m_memWriteTraceList.end()) {
            m_memWriteTraceList.erase(itor);
        }
    }

    template <typename T>
    T vMemRead(Word_t vaddr) {
        Assert(isValidAddr(vaddr));
        T *p = (T *)(m_phyMem + vaddr - MemBase);
        return *p;
    }

    template <typename T>
    void vMemWrite(Word_t vaddr, T data) {
        Assert(isValidAddr(vaddr));
        *(T *)(m_phyMem + vaddr - MemBase) = data;
    }

    template <typename T>
    T vMemReadWithTrace(Word_t vaddr) {
        T data = vMemRead<T>(vaddr);
        traceMemRead(vaddr, data, sizeof(T));
        return data;
    }

    template <typename T>
    void vMemWriteWithTrace(Word_t vaddr, T data) {
        vMemWrite<T>(vaddr, data);
        traceMemWrite(vaddr, data, sizeof(T));
    }

    bool isValidAddr(Word_t vaddr) const {
        return vaddr >= MemBase && vaddr < (MemBase + MemSize);
    }

    bool isValidMemSpan(MemSpan span) const {
        return span.first <= span.second && span.first >= MemBase &&
               span.second < (MemBase + MemSize);
    }
};
}  // namespace remu