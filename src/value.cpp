#include "value.h"
#include "eval_env.h"
#include "error.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <typeinfo>
#include <utility>

double Value::asNumber() const {
    throw LispError("Value is not numeric.");
}
//只接受proper list，即必须以（）结尾
std::vector<std::shared_ptr<Value>> Value::toVector() const {
    if (isNil()) {
        return {};
    }
    
    std::vector<std::shared_ptr<Value>> vec;
    const Value* current = this;
    
    while (current && typeid(*current) == typeid(PairValue)) {
        auto& pair = static_cast<const PairValue&>(*current);
        vec.push_back(pair.getLeft());
        current = pair.getRight().get();
    }
    
    if (current && !current->isNil()) {
        throw LispError("Cannot convert dotted pair to vector");
    }
    
    return vec;
}

BuiltinProcValue::BuiltinProcValue(BuiltinFuncType func) : func{std::move(func)} {}

ValuePtr BuiltinProcValue::apply(const std::vector<ValuePtr>& params, EvalEnv& env) const {
    return func(params, env);
}

std::string BuiltinProcValue::toString() const {
    return "#<procedure>";
}

LambdaValue::LambdaValue(
    std::vector<std::string> params,
    std::vector<ValuePtr> body,
    std::shared_ptr<EvalEnv> env
)
    : params{std::move(params)}, body{std::move(body)}, env{std::move(env)} {}

const std::vector<std::string>& LambdaValue::getParams() const {
    return params;
}

const std::vector<ValuePtr>& LambdaValue::getBody() const {
    return body;
}

ValuePtr LambdaValue::apply(const std::vector<ValuePtr>& args) const {
    if (!env) {
        throw LispError("Lambda has no parent environment.");
    }

    auto localEnv = env->createChild(params, args);
    ValuePtr result = std::make_shared<NilValue>();
    for (const auto& expr : body) {
        result = localEnv->eval(expr);
    }
    return result;
}

std::string LambdaValue::toString() const {
    return "#<procedure>";
}

BooleanValue::BooleanValue(bool value) : value{value} {}
bool BooleanValue::getValue() const { return value; }
std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}

NumericValue::NumericValue(double value) : value{value} {}
double NumericValue::getValue() const { return value; }
std::string NumericValue::toString() const {
    if (std::floor(value) == value) {
        return std::to_string(static_cast<long long>(value));
    }
    return std::to_string(value);
}

StringValue::StringValue(const std::string& value) : value{value} {}
const std::string& StringValue::getValue() const { return value; }
std::string StringValue::toString() const {
    std::ostringstream oss;
    oss << std::quoted(value);
    return oss.str();
}

std::string NilValue::toString() const {
    return "()";
}

SymbolValue::SymbolValue(const std::string& name) : name{name} {}
const std::string& SymbolValue::getName() const { return name; }
std::string SymbolValue::toString() const {
    return name;
}

PairValue::PairValue(ValuePtr left, ValuePtr right) : left{std::move(left)}, right{std::move(right)} {}
ValuePtr PairValue::getLeft() const { return left; }
ValuePtr PairValue::getRight() const { return right; }
std::string PairValue::toString() const {
    std::string result = "(" + left->toString();
    ValuePtr current = right;

    while (current && dynamic_cast<PairValue*>(current.get())) {
        auto& pair = static_cast<const PairValue&>(*current);
        result += " " + pair.getLeft()->toString();
        current = pair.getRight();
    }

    if (!current || current->isNil()) {
        result += ")";
        //列表
    } else {
        result += " . " + current->toString() + ")";
        //带点列表
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
    return os << value.toString();
}
