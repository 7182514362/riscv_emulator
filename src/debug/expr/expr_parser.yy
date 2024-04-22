%require "3.2"
%language "c++"
%header

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
namespace remu {
    class Expr;
}
}

%param { remu::Expr& exp }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "Expr.h"
#include "Memory.h"
#include "Processor.h"
}

%define api.token.prefix {TOK_}
%token
    ASSIGN  "="
    MINUS   "-"
    PLUS    "+"
    STAR    "*"
    SLASH   "/"
    LPAREN  "("
    RPAREN  ")"
    EQ      "=="
    NEQ     "!="
    AND     "&&"
    OR      "||"
    SPACE
    END
;
%token <std::string> REGISTER
%token <unsigned int> NUMBER HEX_NUMBER
%nterm <unsigned int> expr

%printer { yyo << $$; } <*>;

%left "+" "-"
%left "*" "/"
%left EQ NEQ AND OR

%start unit

%%
unit: expr          { exp.result = $1; }

expr: NUMBER        { $$ = $1; }
    | HEX_NUMBER    { $$ = $1; }
    | REGISTER      { $$ = exp.cpu.getGeneralRegFromName($1); }
    | expr "+" expr { $$ = $1 + $3; }
    | expr "-" expr { $$ = $1 - $3; }
    | expr "*" expr { $$ = $1 * $3; }
    | expr "/" expr { $$ = $1 / $3; }
    | "(" expr ")"  { $$ = $2; }
    | expr "==" expr  { $$ = ($1 == $3); }
    | expr "!=" expr { $$ = ($1 != $3); }
    | expr "&&" expr { $$ = ($1 && $3); }
    | expr "||" expr { $$ = ($1 || $3); }
    | "*" expr  { $$ = exp.cpu.getMemory().vMemRead<Word_t>($2); }
    ;
%%

void yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
