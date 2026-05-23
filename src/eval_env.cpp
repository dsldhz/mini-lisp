#include "eval_env.h"
#include "error.h"
#include <vector>

using namespace std::literals;

ValuePtr EvalEnv::eval(ValuePtr expr) {
    if (expr->isSelfEvaluating()) {
        return expr;
    } else if (expr->isNil()) {
        throw LispError("Evaluating nil is prohibited.");
    }
    
    if (auto name = expr->asSymbol()) {
        if (auto it = sym_table.find(*name); it != sym_table.end()) {
            return it->second;
        } else {
            throw LispError("Variable " + *name + " not defined.");
        }
    }
    
    std::vector<ValuePtr> v = expr->toVector();
    if (v.empty()) {
        throw LispError("Evaluating nil is prohibited.");
    }
    
    if (auto name = v[0]->asSymbol(); name == "define"s) {
        if (v.size() != 3) {
            throw LispError("Malformed define.");
        }
        if (auto var_name = v[1]->asSymbol()) {
            sym_table[*var_name] = eval(v[2]);
            return std::make_shared<NilValue>();
        } else {
            throw LispError("Malformed define.");
        }
    }
    
    throw LispError("Unimplemented");
}