%option noyywrap nounput noinput batch debug

%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <cstring> // strerror
# include <string>
# include "Expr.h"
# include "ExprParser.h"

yy::parser::symbol_type
  make_NUMBER (const std::string &s, const yy::parser::location_type& loc);

yy::parser::symbol_type
  make_HEX_NUMBER (const std::string &s, const yy::parser::location_type& loc);
%}

reg     \$[a-zA-Z][a-zA-Z_0-9]*
int     [0-9]+
hex     0[xX][0-9a-fA-F]+
blank   [ \t\r]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%

%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = exp.location;
  // Code run each time yylex is called.
  loc.step ();
%}

{blank}+   loc.step (); return yy::parser::make_SPACE(loc);
[\n\0]    return yy::parser::make_END(loc);

"-"     return yy::parser::make_MINUS(loc);
"+"     return yy::parser::make_PLUS(loc);
"*"     return yy::parser::make_STAR(loc);
"/"     return yy::parser::make_SLASH(loc);
"("     return yy::parser::make_LPAREN(loc);
")"     return yy::parser::make_RPAREN(loc);
"="     return yy::parser::make_ASSIGN(loc);
"=="     return yy::parser::make_EQ(loc);
"!="     return yy::parser::make_NEQ(loc);
"&&"     return yy::parser::make_AND(loc);
"||"     return yy::parser::make_OR(loc);

{int}   return make_NUMBER(yytext, loc);
{hex}   return make_HEX_NUMBER(yytext, loc);
{reg}   return yy::parser::make_REGISTER(yytext, loc);

.   { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }

<<EOF>>    return yy::parser::make_YYEOF (loc);
%%

yy::parser::symbol_type
make_NUMBER (const std::string &s, const yy::parser::location_type& loc)
{
    unsigned int n = std::stoul(s);
    return yy::parser::make_NUMBER (n, loc);
}

yy::parser::symbol_type
make_HEX_NUMBER (const std::string &s, const yy::parser::location_type& loc)
{
    unsigned int n = std::stoul(s, nullptr, 16);
    return yy::parser::make_HEX_NUMBER (n, loc);
}