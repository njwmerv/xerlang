#include "visitor_helper.h"
#include <memory>
#include "../parser/ast.h"

bool is_pointer(const std::string& type) { return type.ends_with('*'); }

bool is_lvalue(const std::unique_ptr<ExprNode>& node) {
    switch (node->node_type) {
        case Parser::ID:
        case Parser::DOT:
        case Parser::ARROW:
            return true;
        case Parser::NUM:
        case Parser::CHARLIT:
        case Parser::TRUE:
        case Parser::FALSE:
        case Parser::NIL:
        case Parser::paramlist:
        case Parser::NEW:
            return false;
        case Parser::OR:
        case Parser::AND:
        case Parser::BITOR:
        case Parser::BITXOR:
        case Parser::BITAND:
        case Parser::EQUALS:
        case Parser::NEQ:
        case Parser::LT:
        case Parser::LEQ:
        case Parser::GT:
        case Parser::GEQ:
        case Parser::LSHIFT:
        case Parser::RSHIFT:
        case Parser::PLUS:
        case Parser::SUB:
        case Parser::MULT:
        case Parser::DIV:
        case Parser::MOD:
        case Parser::EXP: {
            auto* bn = dynamic_cast<BinaryExprNode*>(node.get());
            return is_lvalue(bn->LHS) || is_lvalue(bn->RHS);
        }
        case Parser::AT:
        case Parser::ADDR:
        case Parser::NOT:
        case Parser::BITNOT:
        case Parser::INCR:
        case Parser::DECR:
        case Parser::expr3:
        case Parser::expr4: {
            auto* un = dynamic_cast<UnaryExprNode*>(node.get());
            return is_lvalue(un->arg);
        }
        default:
            return false;
    }
}

bool is_struct(const std::string& type) {
    return !type.starts_with(TYPE_INT) && !type.starts_with(TYPE_CHAR) && !type.starts_with(TYPE_BOOL);
}

bool is_struct_pointer(const std::string& type) {
    if (!is_struct(type)) return false;
    const char secondLast = *std::next(type.rbegin());
    if (secondLast == '*') return false;
    return true;
}

bool is_proc_node(Parser::ParserSymbol ps) {
    return ps == Parser::MAIN || ps == Parser::procedure;
}

void get_parent_nodes(ASTNode* start, ASTNode** prog, ASTNode** proc) {
    ASTNode* curr = start;
    while (curr->parent) {
        if (proc && !*proc && is_proc_node(curr->node_type)) *proc = curr;
        curr = curr->parent;
    }
    if (prog) *prog = curr;
}