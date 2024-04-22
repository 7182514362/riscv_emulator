#include "Command.h"

#include <cstdio>
#include <string>
#include <vector>

#include "Debugger.h"
#include "Expr.h"
#include "Processor.h"

namespace {
std::vector<std::string> stringSplit(const std::string& str, char delim) {
    std::size_t previous = 0;
    std::size_t current = str.find(delim);
    std::vector<std::string> elems;
    while (current != std::string::npos) {
        if (current > previous) {
            elems.push_back(str.substr(previous, current - previous));
        }
        previous = current + 1;
        current = str.find(delim, previous);
    }
    if (previous != str.size()) {
        elems.push_back(str.substr(previous));
    }
    return elems;
}

using CommandEntry = std::pair<std::string_view, remu::CommandKind>;
std::vector<CommandEntry> g_cmdList{
    {"si", remu::CommandKind::STEPI}, {"c", remu::CommandKind::CONTINUE},
    {"p", remu::CommandKind::PRINT},  {"x", remu::CommandKind::EXAMINE},
    {"w", remu::CommandKind::WATCH},  {"b", remu::CommandKind::BREAK},
    {"i", remu::CommandKind::INFO},   {"d", remu::CommandKind::DELETE},
    {"q", remu::CommandKind::QUIT}};

}  // namespace

namespace remu {

CommandKind getCommandKind(const std::string& s) {
    for (auto& pair : g_cmdList) {
        if (s.starts_with(pair.first)) {
            return pair.second;
        }
    }
    return CommandKind::UNKOWN;
}

ICommandUPtr parseCommand(Debugger& d, const std::string& s) {
    auto kind = getCommandKind(s);
    switch (kind) {
        case CommandKind::STEPI:
            return std::make_unique<StepICommand>(d);
            break;
        case CommandKind::PRINT:
            return std::make_unique<PrintCommand>(d);
            break;
        case CommandKind::EXAMINE:
            return std::make_unique<ExamineCommand>(d);
            break;
        case CommandKind::INFO:
            return std::make_unique<InfoCommand>(d);
            break;
        case CommandKind::WATCH:
            return std::make_unique<WatchCommand>(d);
            break;
        case CommandKind::CONTINUE:
            return std::make_unique<ContinueCommand>(d);
            break;
        case CommandKind::BREAK:
            return std::make_unique<BreakCommand>(d);
            break;
        case CommandKind::DELETE:
            return std::make_unique<DeleteCommand>(d);
            break;
        case CommandKind::QUIT:
            return std::make_unique<QuitCommand>(d);
            break;
        case CommandKind::UNKOWN:
        default:
            return nullptr;
            break;
    }
    return nullptr;
}

bool PrintCommand::parse(const std::string& s) {
    if (!s.starts_with("p ")) {
        return false;
    }
    m_expr = s.substr(2);
    return true;
}

int PrintCommand::execute() {
    Expr expr(m_debugger.getProcessor());
    if (expr.parse(m_expr)) {
        std::printf("%s: %d\n", m_expr.data(), expr.result);
    }
    return 0;
}

bool ExamineCommand::parse(const std::string& s) {
    auto list = stringSplit(s, ' ');
    if (list.size() != 3 || list[0] != "x") {
        return false;
    }
    int numOfWord{0};
    Word_t startAddr{0};
    try {
        numOfWord = std::stoi(list[1]);
        Expr exp(m_debugger.getProcessor());
        if (exp.parse(list[2])) {
            startAddr = exp.result;
        } else {
            return false;
        }
    } catch (std::invalid_argument const& ex) {
        std::printf("invalid argument: %s\n", ex.what());
        return false;
    } catch (std::out_of_range const& ex) {
        std::printf("out_of_range: %s\n", ex.what());
        return false;
    }
    MemSpan span{startAddr, startAddr + numOfWord * sizeof(Word_t)};

    if (!m_debugger.getMemory().isValidMemSpan(span)) {
        std::printf("invalid memory span: 0x%08x => 0x%08x\n", span.first,
                    span.second);
        return false;
    }
    m_memSpan = span;
    return true;
}
int ExamineCommand::execute() {
    for (int vaddr = m_memSpan.first; vaddr <= m_memSpan.second; vaddr += 4) {
        Word_t data = m_debugger.getMemory().vMemRead<Word_t>(vaddr);
        uint8_t* bytes = (uint8_t*)(&data);
        std::printf("0x%08x: %02x %02x %02x %02x\n", vaddr, *bytes,
                    *(bytes + 1), *(bytes + 2), *(bytes + 3));
    }
    return 0;
}

bool InfoCommand::parse(const std::string& s) {
    auto list = stringSplit(s, ' ');
    if (list.size() != 2 || list[0] != "i") {
        return false;
    }
    if (list[1].length() != 1) {
        return false;
    }
    m_target = list[1];
    return true;
}

int InfoCommand::execute() {
    if (m_target == "wp") {
        m_debugger.forEachWatchpoint(
            [](const Debugger::Watchpoint& wp) { wp.print(); });
    } else if (m_target == "reg") {
        m_debugger.getProcessor().printGeneralReg();
    } else if (m_target == "bp") {
        m_debugger.forEachBreakpoint(
            [](const Debugger::Breakpoint& bp) { bp.print(); });
    } else {
        std::printf("unkown target %s\n", m_target.data());
    }
    return 0;
}

bool WatchCommand::parse(const std::string& s) {
    auto list = stringSplit(s, ' ');
    if (list.size() != 2 || list[0] != "w") {
        return false;
    }
    Expr expr(m_debugger.getProcessor());
    if (!expr.parse(list[1])) {
        return false;
    }
    Word_t vaddr = expr.result;
    m_addr = vaddr;
    return true;
}

int WatchCommand::execute() {
    m_debugger.addWatchPoint(m_addr);
    return 0;
}

bool BreakCommand::parse(const std::string& s) {
    auto list = stringSplit(s, ' ');
    if (list.size() != 2 || list[0] != "b") {
        return false;
    }
    Expr expr(m_debugger.getProcessor());
    if (!expr.parse(list[1])) {
        return false;
    }
    m_addr = expr.result;
    return true;
}
int BreakCommand::execute() {
    m_debugger.addBreakPoint(m_addr);
    return 0;
}

bool DeleteCommand::parse(const std::string& s) {
    auto list = stringSplit(s, ' ');
    if (list.size() != 3 || list[0] != "d") {
        return false;
    }
    try {
        m_targetId = std::stoi(list[2]);
    } catch (std::invalid_argument const& ex) {
        std::printf("%s\n", ex.what());
        return false;
    } catch (std::out_of_range const& ex) {
        std::printf("%s\n", ex.what());
        return false;
    }
    if (list[1] != "w" && list[1] != "b") {
        return false;
    } else {
        std::printf("unkown delete target %s", list[1].data());
        return false;
    }
    m_target = list[1];
    return true;
}

int DeleteCommand::execute() {
    if (m_target == "w") {
        m_debugger.removeWatchPoint(m_targetId);
    } else if (m_target == "b") {
        m_debugger.removeBreakPoint(m_targetId);
    }
    return 0;
}
}  // namespace remu