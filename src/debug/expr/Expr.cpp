#include "Expr.h"

#include <cstdio>

#include "ExprLexer.h"

namespace remu {
bool Expr::parse(const std::string& exp) {
    YY_BUFFER_STATE handle = yy_scan_string(exp.data());
    if (handle == NULL) {
        return false;
    }
    yy_switch_to_buffer(handle);

    yy::parser parse(*this);
    parse.set_debug_level(false);

    bool result = true;
    try {
        int res = parse();
    } catch (std::invalid_argument const& ex) {
        std::printf("%s\n", ex.what());
        result = false;
    } catch (std::out_of_range const& ex) {
        std::printf("%s\n", ex.what());
        result = false;
    } catch (yy::parser::syntax_error& ex) {
        std::printf("%s\n", ex.what());
        result = false;
    }
    yy_delete_buffer(handle);

    return result;
}
}  // namespace remu
