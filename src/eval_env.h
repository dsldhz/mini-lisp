#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include "./value.h"
#include <unordered_map>
#include <string>

class EvalEnv {
private:
    std::unordered_map<std::string, ValuePtr> sym_table;
public:
    ValuePtr eval(ValuePtr expr);
};

#endif