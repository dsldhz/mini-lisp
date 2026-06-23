#ifndef EVAL_ENV_H
#define EVAL_ENV_H

#include "./value.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class EvalEnv : public std::enable_shared_from_this<EvalEnv> {
private:
    std::unordered_map<std::string, ValuePtr> sym_table;
    std::shared_ptr<EvalEnv> parent;

    explicit EvalEnv(std::shared_ptr<EvalEnv> parent);
    std::vector<ValuePtr> evalList(ValuePtr expr);

public:
    static std::shared_ptr<EvalEnv> createGlobal();
    std::shared_ptr<EvalEnv> createChild(
        const std::vector<std::string>& params,
        const std::vector<ValuePtr>& args
    );
    void defineBinding(const std::string& name, ValuePtr value);
    ValuePtr lookupBinding(const std::string& name) const;
    ValuePtr apply(ValuePtr proc, const std::vector<ValuePtr>& args);
    ValuePtr eval(ValuePtr expr);
};

#endif
