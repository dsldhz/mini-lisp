#ifndef BUILTINS_H
#define BUILTINS_H

#include "./value.h"

#include <string>
#include <unordered_map>

ValuePtr add(const std::vector<ValuePtr>& params);
ValuePtr sub(const std::vector<ValuePtr>& params);
ValuePtr mul(const std::vector<ValuePtr>& params);
ValuePtr div(const std::vector<ValuePtr>& params);
ValuePtr abs(const std::vector<ValuePtr>& params);
ValuePtr expt(const std::vector<ValuePtr>& params);
ValuePtr quotient(const std::vector<ValuePtr>& params);
ValuePtr remainder(const std::vector<ValuePtr>& params);
ValuePtr modulo(const std::vector<ValuePtr>& params);

ValuePtr display(const std::vector<ValuePtr>& params);
ValuePtr exit(const std::vector<ValuePtr>& params);
ValuePtr newline(const std::vector<ValuePtr>& params);
ValuePtr print(const std::vector<ValuePtr>& params);

ValuePtr isAtom(const std::vector<ValuePtr>& params);
ValuePtr isBoolean(const std::vector<ValuePtr>& params);
ValuePtr isInteger(const std::vector<ValuePtr>& params);
ValuePtr isList(const std::vector<ValuePtr>& params);
ValuePtr isNumber(const std::vector<ValuePtr>& params);
ValuePtr isNull(const std::vector<ValuePtr>& params);
ValuePtr isPair(const std::vector<ValuePtr>& params);
ValuePtr isProcedure(const std::vector<ValuePtr>& params);
ValuePtr isString(const std::vector<ValuePtr>& params);
ValuePtr isSymbol(const std::vector<ValuePtr>& params);

ValuePtr car(const std::vector<ValuePtr>& params);
ValuePtr cdr(const std::vector<ValuePtr>& params);
ValuePtr cons(const std::vector<ValuePtr>& params);
ValuePtr length(const std::vector<ValuePtr>& params);
ValuePtr list(const std::vector<ValuePtr>& params);
ValuePtr append(const std::vector<ValuePtr>& params);

ValuePtr numEqual(const std::vector<ValuePtr>& params);
ValuePtr lessThan(const std::vector<ValuePtr>& params);
ValuePtr greaterThan(const std::vector<ValuePtr>& params);
ValuePtr lessEqual(const std::vector<ValuePtr>& params);
ValuePtr greaterEqual(const std::vector<ValuePtr>& params);
ValuePtr isEven(const std::vector<ValuePtr>& params);
ValuePtr isOdd(const std::vector<ValuePtr>& params);
ValuePtr isZero(const std::vector<ValuePtr>& params);

extern const std::unordered_map<std::string, BuiltinFuncType*> builtins;

#endif
