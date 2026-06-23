#include "builtins.h"
#include "eval_env.h"
#include "error.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace {
ValuePtr makeNil() {
    return std::make_shared<NilValue>();
}

ValuePtr makeBoolean(bool value) {
    return std::make_shared<BooleanValue>(value);
}

BuiltinFuncType withoutEnv(ValuePtr (*func)(const std::vector<ValuePtr>&)) {
    return [func](const std::vector<ValuePtr>& params, EvalEnv&) {
        return func(params);
    };
}

void checkArity(const std::string& name, const std::vector<ValuePtr>& params, std::size_t expected) {
    if (params.size() != expected) {
        throw LispError(name + " expects " + std::to_string(expected) + " argument(s).");
    }
}

void checkAtLeast(const std::string& name, const std::vector<ValuePtr>& params, std::size_t min) {
    if (params.size() < min) {
        throw LispError(name + " expects at least " + std::to_string(min) + " argument(s).");
    }
}

double requireNumber(const ValuePtr& value, const std::string& name) {
    if (!value->isNumber()) {
        throw LispError(name + " expects numeric arguments.");
    }
    return value->asNumber();
}

bool isIntegerValue(const ValuePtr& value) {
    return value->isNumber() && std::floor(value->asNumber()) == value->asNumber();
}

long long requireInteger(const ValuePtr& value, const std::string& name) {
    if (!isIntegerValue(value)) {
        throw LispError(name + " expects integer arguments.");
    }
    return static_cast<long long>(value->asNumber());
}

ValuePtr makeList(const std::vector<ValuePtr>& values, ValuePtr tail = makeNil()) {
    auto result = std::move(tail);
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        result = std::make_shared<PairValue>(*it, result);
    }
    return result;
}

bool isProperList(const ValuePtr& value) {
    auto current = value;
    while (auto pair = std::dynamic_pointer_cast<PairValue>(current)) {
        current = pair->getRight();
    }
    return current->isNil();
}

std::size_t listLength(const ValuePtr& value) {
    std::size_t result = 0;
    auto current = value;
    while (auto pair = std::dynamic_pointer_cast<PairValue>(current)) {
        ++result;
        current = pair->getRight();
    }
    if (!current->isNil()) {
        throw LispError("length expects a proper list.");
    }
    return result;
}

std::string displayString(const ValuePtr& value) {
    if (auto string = std::dynamic_pointer_cast<StringValue>(value)) {
        return string->getValue();
    }
    return value->toString();
}

bool isFalse(const ValuePtr& value) {
    auto boolean = std::dynamic_pointer_cast<BooleanValue>(value);
    return boolean && !boolean->getValue();
}

bool eqValue(const ValuePtr& left, const ValuePtr& right) {
    if (left.get() == right.get()) {
        return true;
    }
    if (auto leftNumber = std::dynamic_pointer_cast<NumericValue>(left)) {
        auto rightNumber = std::dynamic_pointer_cast<NumericValue>(right);
        return rightNumber && leftNumber->getValue() == rightNumber->getValue();
    }
    if (auto leftBoolean = std::dynamic_pointer_cast<BooleanValue>(left)) {
        auto rightBoolean = std::dynamic_pointer_cast<BooleanValue>(right);
        return rightBoolean && leftBoolean->getValue() == rightBoolean->getValue();
    }
    if (auto leftSymbol = std::dynamic_pointer_cast<SymbolValue>(left)) {
        auto rightSymbol = std::dynamic_pointer_cast<SymbolValue>(right);
        return rightSymbol && leftSymbol->getName() == rightSymbol->getName();
    }
    return left->isNil() && right->isNil();
}

bool equalValue(const ValuePtr& left, const ValuePtr& right) {
    if (left.get() == right.get()) {
        return true;
    }
    if (auto leftNumber = std::dynamic_pointer_cast<NumericValue>(left)) {
        auto rightNumber = std::dynamic_pointer_cast<NumericValue>(right);
        return rightNumber && leftNumber->getValue() == rightNumber->getValue();
    }
    if (auto leftBoolean = std::dynamic_pointer_cast<BooleanValue>(left)) {
        auto rightBoolean = std::dynamic_pointer_cast<BooleanValue>(right);
        return rightBoolean && leftBoolean->getValue() == rightBoolean->getValue();
    }
    if (auto leftString = std::dynamic_pointer_cast<StringValue>(left)) {
        auto rightString = std::dynamic_pointer_cast<StringValue>(right);
        return rightString && leftString->getValue() == rightString->getValue();
    }
    if (auto leftSymbol = std::dynamic_pointer_cast<SymbolValue>(left)) {
        auto rightSymbol = std::dynamic_pointer_cast<SymbolValue>(right);
        return rightSymbol && leftSymbol->getName() == rightSymbol->getName();
    }
    if (left->isNil()) {
        return right->isNil();
    }
    if (auto leftPair = std::dynamic_pointer_cast<PairValue>(left)) {
        auto rightPair = std::dynamic_pointer_cast<PairValue>(right);
        return rightPair &&
               equalValue(leftPair->getLeft(), rightPair->getLeft()) &&
               equalValue(leftPair->getRight(), rightPair->getRight());
    }
    return false;
}

ValuePtr compareNumbers(const std::vector<ValuePtr>& params, const std::string& name,
                        bool (*predicate)(double, double)) {
    for (const auto& param : params) {
        requireNumber(param, name);
    }
    for (std::size_t i = 1; i < params.size(); ++i) {
        if (!predicate(params[i - 1]->asNumber(), params[i]->asNumber())) {
            return makeBoolean(false);
        }
    }
    return makeBoolean(true);
}
} // namespace

ValuePtr add(const std::vector<ValuePtr>& params) {
    auto result = 0.0;
    for (const auto& param : params) {
        result += requireNumber(param, "+");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr sub(const std::vector<ValuePtr>& params) {
    checkAtLeast("-", params, 1);
    auto result = requireNumber(params[0], "-");
    if (params.size() == 1) {
        return std::make_shared<NumericValue>(-result);
    }
    for (std::size_t i = 1; i < params.size(); ++i) {
        result -= requireNumber(params[i], "-");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr mul(const std::vector<ValuePtr>& params) {
    auto result = 1.0;
    for (const auto& param : params) {
        result *= requireNumber(param, "*");
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr div(const std::vector<ValuePtr>& params) {
    checkAtLeast("/", params, 1);
    auto result = requireNumber(params[0], "/");
    if (params.size() == 1) {
        if (result == 0) {
            throw LispError("Division by zero.");
        }
        return std::make_shared<NumericValue>(1 / result);
    }
    for (std::size_t i = 1; i < params.size(); ++i) {
        auto divisor = requireNumber(params[i], "/");
        if (divisor == 0) {
            throw LispError("Division by zero.");
        }
        result /= divisor;
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr abs(const std::vector<ValuePtr>& params) {
    checkArity("abs", params, 1);
    return std::make_shared<NumericValue>(std::fabs(requireNumber(params[0], "abs")));
}

ValuePtr expt(const std::vector<ValuePtr>& params) {
    checkArity("expt", params, 2);
    return std::make_shared<NumericValue>(
        std::pow(requireNumber(params[0], "expt"), requireNumber(params[1], "expt"))
    );
}

ValuePtr quotient(const std::vector<ValuePtr>& params) {
    checkArity("quotient", params, 2);
    auto dividend = requireInteger(params[0], "quotient");
    auto divisor = requireInteger(params[1], "quotient");
    if (divisor == 0) {
        throw LispError("Division by zero.");
    }
    return std::make_shared<NumericValue>(dividend / divisor);
}

ValuePtr remainder(const std::vector<ValuePtr>& params) {
    checkArity("remainder", params, 2);
    auto dividend = requireInteger(params[0], "remainder");
    auto divisor = requireInteger(params[1], "remainder");
    if (divisor == 0) {
        throw LispError("Division by zero.");
    }
    return std::make_shared<NumericValue>(dividend % divisor);
}

ValuePtr modulo(const std::vector<ValuePtr>& params) {
    checkArity("modulo", params, 2);
    auto dividend = requireInteger(params[0], "modulo");
    auto divisor = requireInteger(params[1], "modulo");
    if (divisor == 0) {
        throw LispError("Division by zero.");
    }
    auto result = dividend % divisor;
    if (result != 0 && ((result > 0) != (divisor > 0))) {
        result += divisor;
    }
    return std::make_shared<NumericValue>(result);
}

ValuePtr applyProc(const std::vector<ValuePtr>& params, EvalEnv& env) {
    checkArity("apply", params, 2);
    return env.apply(params[0], params[1]->toVector());
}

ValuePtr display(const std::vector<ValuePtr>& params) {
    for (const auto& param : params) {
        std::cout << displayString(param);
    }
    return makeNil();
}

ValuePtr displayln(const std::vector<ValuePtr>& params) {
    checkArity("displayln", params, 1);
    std::cout << displayString(params[0]) << std::endl;
    return makeNil();
}

ValuePtr errorProc(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("error");
    }
    checkArity("error", params, 1);
    throw LispError(displayString(params[0]));
}

ValuePtr evalProc(const std::vector<ValuePtr>& params, EvalEnv& env) {
    checkArity("eval", params, 1);
    return env.eval(params[0]);
}

ValuePtr exit(const std::vector<ValuePtr>& params) {
    auto code = 0;
    if (!params.empty()) {
        checkArity("exit", params, 1);
        code = static_cast<int>(requireInteger(params[0], "exit"));
    }
    std::exit(code);
}

ValuePtr newline(const std::vector<ValuePtr>& params) {
    checkArity("newline", params, 0);
    std::cout << std::endl;
    return makeNil();
}

ValuePtr print(const std::vector<ValuePtr>& params) {
    for (const auto& param : params) {
        std::cout << param->toString() << std::endl;
    }
    return makeNil();
}

ValuePtr isAtom(const std::vector<ValuePtr>& params) {
    checkArity("atom?", params, 1);
    return makeBoolean(
        !std::dynamic_pointer_cast<PairValue>(params[0]) &&
        !std::dynamic_pointer_cast<BuiltinProcValue>(params[0]) &&
        !std::dynamic_pointer_cast<LambdaValue>(params[0])
    );
}

ValuePtr isBoolean(const std::vector<ValuePtr>& params) {
    checkArity("boolean?", params, 1);
    return makeBoolean(static_cast<bool>(std::dynamic_pointer_cast<BooleanValue>(params[0])));
}

ValuePtr isInteger(const std::vector<ValuePtr>& params) {
    checkArity("integer?", params, 1);
    return makeBoolean(isIntegerValue(params[0]));
}

ValuePtr isList(const std::vector<ValuePtr>& params) {
    checkArity("list?", params, 1);
    return makeBoolean(isProperList(params[0]));
}

ValuePtr isNumber(const std::vector<ValuePtr>& params) {
    checkArity("number?", params, 1);
    return makeBoolean(params[0]->isNumber());
}

ValuePtr isNull(const std::vector<ValuePtr>& params) {
    checkArity("null?", params, 1);
    return makeBoolean(params[0]->isNil());
}

ValuePtr isPair(const std::vector<ValuePtr>& params) {
    checkArity("pair?", params, 1);
    return makeBoolean(static_cast<bool>(std::dynamic_pointer_cast<PairValue>(params[0])));
}

ValuePtr isProcedure(const std::vector<ValuePtr>& params) {
    checkArity("procedure?", params, 1);
    return makeBoolean(
        static_cast<bool>(std::dynamic_pointer_cast<BuiltinProcValue>(params[0])) ||
        static_cast<bool>(std::dynamic_pointer_cast<LambdaValue>(params[0]))
    );
}

ValuePtr isString(const std::vector<ValuePtr>& params) {
    checkArity("string?", params, 1);
    return makeBoolean(static_cast<bool>(std::dynamic_pointer_cast<StringValue>(params[0])));
}

ValuePtr isSymbol(const std::vector<ValuePtr>& params) {
    checkArity("symbol?", params, 1);
    return makeBoolean(params[0]->asSymbol().has_value());
}

ValuePtr car(const std::vector<ValuePtr>& params) {
    checkArity("car", params, 1);
    if (auto pair = std::dynamic_pointer_cast<PairValue>(params[0])) {
        return pair->getLeft();
    }
    throw LispError("car expects a pair.");
}

ValuePtr cdr(const std::vector<ValuePtr>& params) {
    checkArity("cdr", params, 1);
    if (auto pair = std::dynamic_pointer_cast<PairValue>(params[0])) {
        return pair->getRight();
    }
    throw LispError("cdr expects a pair.");
}

ValuePtr cons(const std::vector<ValuePtr>& params) {
    checkArity("cons", params, 2);
    return std::make_shared<PairValue>(params[0], params[1]);
}

ValuePtr length(const std::vector<ValuePtr>& params) {
    checkArity("length", params, 1);
    return std::make_shared<NumericValue>(static_cast<double>(listLength(params[0])));
}

ValuePtr list(const std::vector<ValuePtr>& params) {
    return makeList(params);
}

ValuePtr append(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        return makeNil();
    }

    auto result = params.back();
    for (auto it = params.rbegin() + 1; it != params.rend(); ++it) {
        auto values = (*it)->toVector();
        result = makeList(values, result);
    }
    return result;
}

ValuePtr mapProc(const std::vector<ValuePtr>& params, EvalEnv& env) {
    checkArity("map", params, 2);
    std::vector<ValuePtr> result;
    for (const auto& item : params[1]->toVector()) {
        result.push_back(env.apply(params[0], {item}));
    }
    return makeList(result);
}

ValuePtr filterProc(const std::vector<ValuePtr>& params, EvalEnv& env) {
    checkArity("filter", params, 2);
    std::vector<ValuePtr> result;
    for (const auto& item : params[1]->toVector()) {
        if (!isFalse(env.apply(params[0], {item}))) {
            result.push_back(item);
        }
    }
    return makeList(result);
}

ValuePtr reduceProc(const std::vector<ValuePtr>& params, EvalEnv& env) {
    checkArity("reduce", params, 2);
    auto values = params[1]->toVector();
    if (values.empty()) {
        throw LispError("reduce expects a non-empty list.");
    }

    auto result = values.back();
    for (std::size_t i = values.size() - 1; i > 0; --i) {
        result = env.apply(params[0], {values[i - 1], result});
    }
    return result;
}

ValuePtr isEq(const std::vector<ValuePtr>& params) {
    checkArity("eq?", params, 2);
    return makeBoolean(eqValue(params[0], params[1]));
}

ValuePtr isEqual(const std::vector<ValuePtr>& params) {
    checkArity("equal?", params, 2);
    return makeBoolean(equalValue(params[0], params[1]));
}

ValuePtr notProc(const std::vector<ValuePtr>& params) {
    checkArity("not", params, 1);
    return makeBoolean(isFalse(params[0]));
}

ValuePtr numEqual(const std::vector<ValuePtr>& params) {
    return compareNumbers(params, "=", [](double left, double right) { return left == right; });
}

ValuePtr lessThan(const std::vector<ValuePtr>& params) {
    return compareNumbers(params, "<", [](double left, double right) { return left < right; });
}

ValuePtr greaterThan(const std::vector<ValuePtr>& params) {
    return compareNumbers(params, ">", [](double left, double right) { return left > right; });
}

ValuePtr lessEqual(const std::vector<ValuePtr>& params) {
    return compareNumbers(params, "<=", [](double left, double right) { return left <= right; });
}

ValuePtr greaterEqual(const std::vector<ValuePtr>& params) {
    return compareNumbers(params, ">=", [](double left, double right) { return left >= right; });
}

ValuePtr isEven(const std::vector<ValuePtr>& params) {
    checkArity("even?", params, 1);
    return makeBoolean(requireInteger(params[0], "even?") % 2 == 0);
}

ValuePtr isOdd(const std::vector<ValuePtr>& params) {
    checkArity("odd?", params, 1);
    return makeBoolean(requireInteger(params[0], "odd?") % 2 != 0);
}

ValuePtr isZero(const std::vector<ValuePtr>& params) {
    checkArity("zero?", params, 1);
    return makeBoolean(requireNumber(params[0], "zero?") == 0);
}

const std::unordered_map<std::string, BuiltinFuncType> builtins{
    {"+", withoutEnv(add)},
    {"-", withoutEnv(sub)},
    {"*", withoutEnv(mul)},
    {"/", withoutEnv(div)},
    {"abs", withoutEnv(abs)},
    {"expt", withoutEnv(expt)},
    {"quotient", withoutEnv(quotient)},
    {"remainder", withoutEnv(remainder)},
    {"modulo", withoutEnv(modulo)},

    {"apply", applyProc},
    {"display", withoutEnv(display)},
    {"displayln", withoutEnv(displayln)},
    {"error", withoutEnv(errorProc)},
    {"eval", evalProc},
    {"exit", withoutEnv(exit)},
    {"newline", withoutEnv(newline)},
    {"print", withoutEnv(print)},
    
    {"atom?", withoutEnv(isAtom)},
    {"boolean?", withoutEnv(isBoolean)},
    {"integer?", withoutEnv(isInteger)},
    {"list?", withoutEnv(isList)},
    {"number?", withoutEnv(isNumber)},
    {"null?", withoutEnv(isNull)},
    {"pair?", withoutEnv(isPair)},
    {"procedure?", withoutEnv(isProcedure)},
    {"string?", withoutEnv(isString)},
    {"symbol?", withoutEnv(isSymbol)},

    {"car", withoutEnv(car)},
    {"cdr", withoutEnv(cdr)},
    {"cons", withoutEnv(cons)},
    {"length", withoutEnv(length)},
    {"list", withoutEnv(list)},
    {"append", withoutEnv(append)},
    {"map", mapProc},
    {"filter", filterProc},
    {"reduce", reduceProc},

    {"eq?", withoutEnv(isEq)},
    {"equal?", withoutEnv(isEqual)},
    {"not", withoutEnv(notProc)},
    {"=", withoutEnv(numEqual)},
    {"<", withoutEnv(lessThan)},
    {">", withoutEnv(greaterThan)},
    {"<=", withoutEnv(lessEqual)},
    {">=", withoutEnv(greaterEqual)},
    {"even?", withoutEnv(isEven)},
    {"odd?", withoutEnv(isOdd)},
    {"zero?", withoutEnv(isZero)},
};
