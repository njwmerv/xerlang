#ifndef PARSER_H
#define PARSER_H

#include <stack>
#include "../util/types.h"
#include "ast.h"

class Parser{
    std::stack<Symbol> symbolStack;
    std::stack<ParserDFAState> stateStack;

    static Parser* instance;

    Parser();

public:
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = delete;
    Parser& operator=(Parser&&) = delete;

    static Parser* getInstance();

    AST* parse(std::ifstream& tokenStream);
};

#endif //PARSER_H
