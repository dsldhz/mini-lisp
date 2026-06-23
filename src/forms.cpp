#include "./forms.h"
#include "./error.h"
#include "./eval_env.h"

#include <memory>
#include <utility>

namespace {
ValuePtr makeNil() {
    return std::make_shared<NilValue>();
}

ValuePtr makeBoolean(bool value) {
    return std::make_shared<BooleanValue>(value);
}

void checkArity(const std::string& name, const std::vector<ValuePtr>& args, std::size_t expected) {
    if (args.size() != expected) {
        throw LispError(name + " expects " + std::to_string(expected) + " argument(s).");
    }
}

void checkAtLeast(const std::string& name, const std::vector<ValuePtr>& args, std::size_t min) {
    if (args.size() < min) {
        throw LispError(name + " expects at least " + std::to_string(min) + " argument(s).");
    }
}

bool isFalse(const ValuePtr& value) {
    auto boolean = std::dynamic_pointer_cast<BooleanValue>(value);
    return boolean && !boolean->getValue();
}

std::vector<std::string> parseParamVector(const std::vector<ValuePtr>& params) {
    std::vector<std::string> result;
    for (const auto& param : params) {
        if (auto name = param->asSymbol()) {
            result.push_back(*name);
        } else {
            throw LispError("lambda expects symbols in parameter list.");
        }
    }
    return result;
}

std::vector<std::string> parseParams(const ValuePtr& value) {
    return parseParamVector(value->toVector());
}

ValuePtr makeLambda(const ValuePtr& paramsExpr, std::vector<ValuePtr> body, EvalEnv& env) {
    if (body.empty()) {
        throw LispError("lambda expects a body.");
    }
    return std::make_shared<LambdaValue>(parseParams(paramsExpr), std::move(body), env.shared_from_this());
}
} // namespace

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    (void)env;
    checkArity("quote", args, 1);
    return args[0];
}

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    checkAtLeast("define", args, 2);

    if (auto name = args[0]->asSymbol()) {
        checkArity("define", args, 2);
        env.defineBinding(*name, env.eval(args[1]));
        //右侧表达式会立即在当前环境中求值，然后绑定结果。
        return makeNil();
    }

    auto signature = args[0]->toVector();
    if (signature.empty()) {
        throw LispError("Malformed define.");
    }

    auto name = signature[0]->asSymbol();
    if (!name) {
        throw LispError("Malformed define.");
    }

    std::vector<ValuePtr> params(signature.begin() + 1, signature.end());
    std::vector<ValuePtr> body(args.begin() + 1, args.end());
    env.defineBinding(
        *name,
        std::make_shared<LambdaValue>(
            parseParamVector(params),
            std::move(body),
            env.shared_from_this()
        )
    );
    return makeNil();
}

ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    checkArity("if", args, 3);
    return env.eval(isFalse(env.eval(args[0])) ? args[2] : args[1]);
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    ValuePtr result = makeBoolean(true);
    for (const auto& arg : args) {
        result = env.eval(arg);
        if (isFalse(result)) {
            return result;
        }
    }
    return result;
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for (const auto& arg : args) {
        auto result = env.eval(arg);
        if (!isFalse(result)) {
            return result;
        }
    }
    return makeBoolean(false);
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    checkAtLeast("lambda", args, 2);
    std::vector<ValuePtr> body(args.begin() + 1, args.end());
    return makeLambda(args[0], std::move(body), env);
}

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"quote", &quoteForm},
    {"define", &defineForm},
    {"if", &ifForm},
    {"and", &andForm},
    {"or", &orForm},
    {"lambda", &lambdaForm},
};
