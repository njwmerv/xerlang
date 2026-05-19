#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "util/types.h"

std::unique_ptr<ASTNode> parse(const std::vector<Token>& stream, std::ostream& err);

#endif //PARSER_H
