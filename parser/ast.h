#ifndef AST_H
#define AST_H

#include "../util/types.h"
#include <vector>
#include <optional>

class AST{
    struct Node{
        Node* parent;
        Symbol symbol;
        std::optional<std::string> lexeme;
        std::vector<Node*> children;

        bool isTerminal() const;

        ~Node();
    };

    Node* root;

    ~AST();
};

#endif //AST_H
