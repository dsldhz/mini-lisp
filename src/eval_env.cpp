#include "eval_env.h"
#include "builtins.h"
#include "error.h"

#include <memory>
#include <vector>

using namespace std::literals;

EvalEnv::EvalEnv() {
    for (const auto& [name, func] : builtins) {
        sym_table[name] = std::make_shared<BuiltinProcValue>(func);
    }
}

std::vector<ValuePtr> EvalEnv::evalList(ValuePtr expr) {
    std::vector<ValuePtr> result;
    for (const auto& item : expr->toVector()) {
        result.push_back(eval(item));
    }
    return result;
}

ValuePtr EvalEnv::apply(ValuePtr proc, const std::vector<ValuePtr>& args) {
    if (auto builtin = std::dynamic_pointer_cast<BuiltinProcValue>(proc)) {
        return builtin->apply(args);
    }
    throw LispError("Unimplemented");
}

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
    
    if (auto name = v[0]->asSymbol(); name == "quote"s) {
        if (v.size() != 2) {
            throw LispError("Malformed quote.");
        }
        return v[1];
    } else if (name == "define"s) {
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

    auto pair = std::dynamic_pointer_cast<PairValue>(expr);
    auto proc = eval(pair->getLeft());
    auto args = evalList(pair->getRight());
    return apply(proc, args);
}
