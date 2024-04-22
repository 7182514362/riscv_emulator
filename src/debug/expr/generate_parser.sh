#!/bin/bash


# bison --debug -d --header=expr_parser.h -o expr_parser.c expr_parser.y
# flex --debug --header-file=expr_lexer.h -o expr_lexer.c expr_lexer.l

bison -d --header=ExprParser.h -o ExprParser.cpp expr_parser.yy
flex --header-file=ExprLexer.h -o ExprLexer.cpp expr_lexer.ll