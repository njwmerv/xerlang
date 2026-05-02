#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "util/types.h"

struct ParsingTableEntry {
    enum class Action : uint8_t { NIL, SHIFT, REDUCE, GOTO, ACCEPT };

    Action act = Action::NIL;
    union {
        uint16_t target_state = 0;
        uint16_t production_id;
    };
};

std::unique_ptr<ASTNode> parse(std::vector<Token>& stream);

void print_AST(const std::unique_ptr<ASTNode>& root, size_t depth, std::ostream& os);

#endif //PARSER_H
