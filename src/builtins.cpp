#include "builtins.h"
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

ValuePtr display(const std::vector<ValuePtr>& params) {
    for (const auto& param : params) {
        std::cout << displayString(param);
    }
    return makeNil();
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
        !std::dynamic_pointer_cast<BuiltinProcValue>(params[0])
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
    return makeBoolean(static_cast<bool>(std::dynamic_pointer_cast<BuiltinProcValue>(params[0])));
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

const std::unordered_map<std::string, BuiltinFuncType*> builtins{
    {"+", &add},
    {"-", &sub},
    {"*", &mul},
    {"/", &div},
    {"abs", &abs},
    {"expt", &expt},
    {"quotient", &quotient},
    {"remainder", &remainder},
    {"modulo", &modulo},

    {"display", &display},
    {"exit", &exit},
    {"newline", &newline},
    {"print", &print},
    
    {"atom?", &isAtom},
    {"boolean?", &isBoolean},
    {"integer?", &isInteger},
    {"list?", &isList},
    {"number?", &isNumber},
    {"null?", &isNull},
    {"pair?", &isPair},
    {"procedure?", &isProcedure},
    {"string?", &isString},
    {"symbol?", &isSymbol},

    {"car", &car},
    {"cdr", &cdr},
    {"cons", &cons},
    {"length", &length},
    {"list", &list},
    {"append", &append},

    {"=", &numEqual},
    {"<", &lessThan},
    {">", &greaterThan},
    {"<=", &lessEqual},
    {">=", &greaterEqual},
    {"even?", &isEven},
    {"odd?", &isOdd},
    {"zero?", &isZero},
};
