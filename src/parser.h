#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "value.h"
#include <deque>

class Parser {
public:
    Parser(std::deque<TokenPtr> tokens);
    ValuePtr parse();
private:
    std::deque<TokenPtr> tokens;
    ValuePtr parseTails();
};

#endif
