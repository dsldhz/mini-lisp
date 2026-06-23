#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"

#include "rjsj_test.hpp"

struct TestCtx {
    std::shared_ptr<EvalEnv> env = EvalEnv::createGlobal();

    std::string eval(std::string input) {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        auto value = parser.parse();
        return env->eval(std::move(value))->toString();
    }
};

namespace {
std::vector<ValuePtr> parseProgram(const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    return parser.parseAll();
}

ValuePtr evalOne(const std::shared_ptr<EvalEnv>& env, const std::string& input) {
    auto tokens = Tokenizer::tokenize(input);
    Parser parser(std::move(tokens));
    return env->eval(parser.parse());
}

void runRepl() {
    auto env = EvalEnv::createGlobal();
    while (true) {
        try {
            std::cout << ">>> " ;
            std::string line;
            std::getline(std::cin, line);
            if (std::cin.eof()) {
                std::exit(0);
            }
            auto result = evalOne(env, line);
            std::cout << result->toString() << std::endl;
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int runFile(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Error: Cannot open file " << path << std::endl;
        return 1;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    try {
        auto env = EvalEnv::createGlobal();
        for (const auto& expr : parseProgram(buffer.str())) {
            env->eval(expr);
        }
    } catch (std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
} // namespace

int main(int argc, char* argv[]) {
    //RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6, Lv7, Lv7Lib, Sicp);

    if (argc > 1) {
        return runFile(argv[1]);
    }
    runRepl();
    return 0;
}
