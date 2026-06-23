#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "value.h"
#include <deque>
#include <vector>

class Parser {
public:
    Parser(std::deque<TokenPtr> tokens);
    ValuePtr parse();
    std::vector<ValuePtr> parseAll();
private:
    std::deque<TokenPtr> tokens;
    ValuePtr parseTails();
};

#endif
