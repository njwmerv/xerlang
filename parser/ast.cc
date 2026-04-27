#include "ast.h"

AST::Node::~Node(){
    parent = nullptr;
    for (auto child : children) {
        delete child;
    }
}

AST::Node::isTerminal() const {
    return lexeme.has_value();
}

AST::~AST(){
    delete root;
}