#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "Debugger.h"
#include "Memory.h"
#include "Processor.h"

namespace remu {
enum class CommandKind {
    UNKOWN,
    STEPI,
    PRINT,
    EXAMINE,
    INFO,
    WATCH,
    CONTINUE,
    BREAK,
    DELETE,
    QUIT
};

class ICommand {
protected:
    Debugger& m_debugger;

public:
    explicit ICommand(Debugger& d) : m_debugger(d) {}
    virtual ~ICommand() {}

    virtual bool parse(const std::string&) = 0;
    // return number of instruction to execute
    virtual int execute() = 0;
};

using ICommandUPtr = std::unique_ptr<remu::ICommand>;

class StepICommand : public ICommand {
public:
    StepICommand(Debugger& d) : ICommand(d) {}
    ~StepICommand() = default;

    bool parse(const std::string& s) override { return s == "si"; }
    int execute() override { return 1; }
};

class ContinueCommand : public ICommand {
public:
    ContinueCommand(Debugger& d) : ICommand(d) {}
    ~ContinueCommand() = default;

    bool parse(const std::string& s) override { return s == "c"; }
    int execute() override { return INT32_MAX; }
};

class PrintCommand : public ICommand {
private:
    std::string m_expr;

public:
    PrintCommand(Debugger& d) : ICommand(d) {}
    ~PrintCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class ExamineCommand : public ICommand {
private:
    MemSpan m_memSpan;

public:
    ExamineCommand(Debugger& d) : ICommand(d), m_memSpan({0, 0}) {}
    ~ExamineCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class InfoCommand : public ICommand {
private:
    // 'wp'/'reg'/'bp'
    std::string m_target;

public:
    InfoCommand(Debugger& d) : ICommand(d), m_target(0) {}
    ~InfoCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class WatchCommand : public ICommand {
private:
    Word_t m_addr;

public:
    WatchCommand(Debugger& d) : ICommand(d), m_addr(0) {}
    ~WatchCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class BreakCommand : public ICommand {
private:
    Word_t m_addr;

public:
    BreakCommand(Debugger& d) : ICommand(d), m_addr(0) {}
    ~BreakCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class DeleteCommand : public ICommand {
private:
    std::string m_target;
    int m_targetId;

public:
    DeleteCommand(Debugger& d) : ICommand(d), m_targetId(-1) {}
    ~DeleteCommand() = default;

    bool parse(const std::string&) override;
    int execute() override;
};

class QuitCommand : public ICommand {
public:
    QuitCommand(Debugger& d) : ICommand(d) {}
    ~QuitCommand() = default;

    bool parse(const std::string& s) override {
        return s == "q" || s == "quit";
    }
    int execute() override {
        m_debugger.quit();
        return 0;
    }
};

CommandKind getCommandKind(const std::string& s);

ICommandUPtr parseCommand(Debugger& d, const std::string& s);
};  // namespace remu