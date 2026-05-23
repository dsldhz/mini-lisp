#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include "./value.h"
#include <unordered_map>
#include <string>
#include <vector>

class EvalEnv {
private:
    std::unordered_map<std::string, ValuePtr> sym_table;
    std::vector<ValuePtr> evalList(ValuePtr expr);
    ValuePtr apply(ValuePtr proc, const std::vector<ValuePtr>& args);
public:
    EvalEnv();
    ValuePtr eval(ValuePtr expr);
};

#endif
