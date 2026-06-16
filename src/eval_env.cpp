#include "eval_env.h"
#include "builtins.h"
#include "error.h"
#include "forms.h"

#include <memory>
#include <utility>
#include <vector>

EvalEnv::EvalEnv(std::shared_ptr<EvalEnv> parent) : parent{std::move(parent)} {
    if (!this->parent) {
        for (const auto& [name, func] : builtins) {
            sym_table[name] = std::make_shared<BuiltinProcValue>(func);
        }
    }
}

std::shared_ptr<EvalEnv> EvalEnv::createGlobal() {
    return std::shared_ptr<EvalEnv>(new EvalEnv(nullptr));
}

std::shared_ptr<EvalEnv> EvalEnv::createChild(
    const std::vector<std::string>& params,
    const std::vector<ValuePtr>& args
) {
    if (params.size() != args.size()) {
        throw LispError("Procedure expects " + std::to_string(params.size()) + " argument(s).");
    }

    auto child = std::shared_ptr<EvalEnv>(new EvalEnv(shared_from_this()));
    for (std::size_t i = 0; i < params.size(); ++i) {
        child->defineBinding(params[i], args[i]);
    }
    return child;
}

void EvalEnv::defineBinding(const std::string& name, ValuePtr value) {
    sym_table[name] = std::move(value);
}

ValuePtr EvalEnv::lookupBinding(const std::string& name) const {
    if (auto it = sym_table.find(name); it != sym_table.end()) {
        return it->second;
    }
    if (parent) {
        return parent->lookupBinding(name);
    }
    throw LispError("Variable " + name + " not defined.");
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
    if (auto lambda = std::dynamic_pointer_cast<LambdaValue>(proc)) {
        return lambda->apply(args);
    }
    throw LispError("Cannot apply non-procedure.");
}

ValuePtr EvalEnv::eval(ValuePtr expr) {
    if (expr->isSelfEvaluating()) {
        return expr;
    } else if (expr->isNil()) {
        throw LispError("Evaluating nil is prohibited.");
    }
    
    if (auto name = expr->asSymbol()) {
        return lookupBinding(*name);
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
