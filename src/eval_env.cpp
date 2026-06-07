#include "eval_env.h"
#include "builtins.h"
#include "error.h"
#include "forms.h"

#include <memory>
#include <utility>
#include <vector>

EvalEnv::EvalEnv() {
    for (const auto& [name, func] : builtins) {
        sym_table[name] = std::make_shared<BuiltinProcValue>(func);
    }
}

void EvalEnv::defineBinding(const std::string& name, ValuePtr value) {
    sym_table[name] = std::move(value);
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

    auto pair = std::dynamic_pointer_cast<PairValue>(expr);
    if (!pair) {
        throw LispError("Malformed expression.");
    }

    if (auto name = pair->getLeft()->asSymbol()) {
        if (auto it = SPECIAL_FORMS.find(*name); it != SPECIAL_FORMS.end()) {
            return it->second(pair->getRight()->toVector(), *this);
        }
    }

    auto proc = eval(pair->getLeft());
    auto args = evalList(pair->getRight());
    return apply(proc, args);
}
