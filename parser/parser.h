#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "util/types.h"

std::unique_ptr<ASTNode> parse(const std::vector<Token>& stream, std::ostream& err);

void print_AST(const std::unique_ptr<ASTNode>& root, size_t depth, std::ostream& os);

#endif //PARSER_H
