#pragma once

#include <stdexcept>
#include <string>

#include "ExprParser.h"
#include "Processor.h"

#define YY_DECL yy::parser::symbol_type yylex(remu::Expr& exp)
// ... and declare it for the parser's sake.
YY_DECL;

namespace remu {
class Expr {
public:
    unsigned int result;
    yy::location location;

    Processor& cpu;

    explicit Expr(Processor& p) : result(0), cpu(p) {}
    ~Expr() {}

    bool parse(const std::string& exp);
};
}  // namespace remu