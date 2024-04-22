include_directories(${CMAKE_CURRENT_SOURCE_DIR}/expr)

add_library(debugger STATIC Debugger.cpp  Command.cpp expr/Expr.cpp expr/ExprParser.cpp
                            expr/ExprLexer.cpp)
