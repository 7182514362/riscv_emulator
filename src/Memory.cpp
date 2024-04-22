#include "Memory.h"

namespace remu {
void Memory::traceMemRead(Word_t vaddr, Word_t data, int numOfBytes) {
    for (auto t : m_memReadTraceList) {
        if (!t.inSpan(vaddr)) {
            continue;
        }
        t(vaddr, data, numOfBytes);
    }
}

void Memory::traceMemWrite(Word_t vaddr, Word_t data, int numOfbytes) {
    for (auto t : m_memWriteTraceList) {
        if (!t.inSpan(vaddr)) {
            continue;
        }
        t(vaddr, data, numOfbytes);
    }
}
}  // namespace remu