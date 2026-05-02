#include "parser.h"
#include <iostream>
#include <vector>
#include <memory>
#include "parser_constants.h"

using namespace Parser;

std::ostream& operator<<(std::ostream& os, Parser::ParserSymbol state) {
    switch (state) {
        case MAIN: os << "MAIN"; break;
        case READ: os << "READ"; break;
        case PRINT: os << "PRINT"; break;
        case INT: os << "INT"; break;
        case CHAR: os << "CHAR"; break;
        case BOOL: os << "BOOL"; break;
        case STRUCT: os << "STRUCT"; break;
        case TRUE: os << "TRUE"; break;
        case FALSE: os << "FALSE"; break;
        case NIL: os << "NIL"; break;
        case NUM: os << "NUM"; break;
        case CHARLIT: os << "CHARLIT"; break;
        case ID: os << "ID"; break;
        case RETURN: os << "RETURN"; break;
        case IF: os << "IF"; break;
        case ELIF: os << "ELIF"; break;
        case ELSE: os << "ELSE"; break;
        case FOR: os << "FOR"; break;
        case WHILE: os << "WHILE"; break;
        case BREAK: os << "BREAK"; break;
        case DELETE: os << "DELETE"; break;
        case NEW: os << "NEW"; break;
        case COLON: os << "COLON"; break;
        case LPAREN: os << "LPAREN"; break;
        case RPAREN: os << "RPAREN"; break;
        case SEMI: os << "SEMI"; break;
        case LCURLY: os << "LCURLY"; break;
        case RCURLY: os << "RCURLY"; break;
        case COMMA: os << "COMMA"; break;
        case LBRACK: os << "LBRACK"; break;
        case RBRACK: os << "RBRACK"; break;
        case BECOMES: os << "BECOMES"; break;
        case NOT: os << "NOT"; break;
        case OR: os << "OR"; break;
        case AND: os << "AND"; break;
        case GEQ: os << "GEQ"; break;
        case GT: os << "GT"; break;
        case LEQ: os << "LEQ"; break;
        case LT: os << "LT"; break;
        case EQUALS: os << "EQUALS"; break;
        case NEQ: os << "NEQ"; break;
        case PLUS: os << "PLUS"; break;
        case SUB: os << "SUB"; break;
        case MULT: os << "MULT"; break;
        case AT: os << "AT"; break;
        case ADDR: os << "ADDR"; break;
        case DIV: os << "DIV"; break;
        case MOD: os << "MOD"; break;
        case LSHIFT: os << "LSHIFT"; break;
        case RSHIFT: os << "RSHIFT"; break;
        case EXP: os << "EXP"; break;
        case BITOR: os << "BITOR"; break;
        case BITXOR: os << "BITXOR"; break;
        case BITAND: os << "BITAND"; break;
        case BITNOT: os << "BITNOT"; break;
        case INCR: os << "INCR"; break;
        case DECR: os << "DECR"; break;
        case ARROW: os << "ARROW"; break;
        case DOT: os << "DOT"; break;
        case BoF: os << "BoF"; break;
        case EoF: os << "EoF"; break;
        case start: os << "start"; break;
        case procedures: os << "procedures"; break;
        case procedure: os << "procedure"; break;
        case main: os << "main"; break;
        case type: os << "type"; break;
        case star: os << "star"; break;
        case params: os << "params"; break;
        case dcls: os << "dcls"; break;
        case dcl: os << "dcl"; break;
        case statements: os << "statements"; break;
        case statement: os << "statement"; break;
        case ifs: os << "ifs"; break;
        case forprologue: os << "forprologue"; break;
        case forepilogue: os << "forepilogue"; break;
        case expr1: os << "expr1"; break;
        case expr2: os << "expr2"; break;
        case expr3: os << "expr3"; break;
        case expr4: os << "expr4"; break;
        case expr5: os << "expr5"; break;
        case expr6: os << "expr6"; break;
        case expr7: os << "expr7"; break;
        case expr8: os << "expr8"; break;
        case expr9: os << "expr9"; break;
        case expr10: os << "expr10"; break;
        case expr11: os << "expr11"; break;
        case expr12: os << "expr12"; break;
        case expr13: os << "expr13"; break;
        case expr14: os << "expr14"; break;
        case args: os << "args"; break;
        case structdef: os << "structdef"; break;
        default: os << "NONE"; break;
    }
    return os;
}

struct LALRData {
    uint16_t state = 0;
    std::unique_ptr<ASTNode> data = nullptr;
};

std::unique_ptr<ASTNode> create_leaf(const Token& token) {
    std::unique_ptr<ASTNode> ptr = std::make_unique<ASTNode>();
    ptr->node_type = token.type;
    ptr->lexeme = token.lexeme;
    return ptr;
}

std::unique_ptr<ASTNode> reduce_nodes(const Production& prod, std::vector<std::unique_ptr<ASTNode>>&& children) {
    std::unique_ptr<ASTNode> ptr = std::make_unique<ASTNode>();
    ptr->production = prod;
    ptr->node_type = prod.LHS;
    ptr->children = std::move(children);
    for (auto& child : ptr->children) child->parent = ptr.get();
    return ptr;
}

std::vector<LALRData> stack;

std::unique_ptr<ASTNode> parse(std::vector<Token>& stream) {
    if (!stack.empty()) stack.clear();
    stack.push_back({0, nullptr});
    size_t token_idx = 0;

    while (token_idx < stream.size()) {
        const Token& lookahead = stream.at(token_idx);
        uint16_t current_state = stack.back().state;

        const ParsingTableEntry& pte = PARSING_TABLE[current_state][lookahead.type];

        switch (pte.act) {
            case ParsingTableEntry::Action::SHIFT:
                stack.push_back({pte.target_state, create_leaf(lookahead)});
                token_idx++;
                break;
            case ParsingTableEntry::Action::REDUCE: {
                const Production& prod = PRODUCTIONS[pte.production_id];
                std::vector<std::unique_ptr<ASTNode>> children;

                int i;
                for(i = 0; i < prod.len; i++)
                    children.push_back(std::move(stack.at(stack.size() - prod.len + i).data));
                for(i = 0; i < prod.len; i++)
                    stack.pop_back();

                if(stack.empty()) throw std::runtime_error{"ERROR: Attempting to read empty stack!"};
                uint16_t state_after_pop = stack.back().state;

                const ParsingTableEntry& goto_pte = PARSING_TABLE[state_after_pop][prod.LHS];
                if(goto_pte.act != ParsingTableEntry::Action::GOTO) throw std::runtime_error{"ERROR: NOT GOTO entry found!"};

                stack.push_back({goto_pte.target_state, reduce_nodes(prod, std::move(children))});
                break;
            }
            case ParsingTableEntry::Action::ACCEPT:
                return std::move(stack.back().data);
            default:
                throw std::runtime_error{"ERROR: Default - NO valid parse possible."};
        }
    }
    throw std::runtime_error{"ERROR: Out - NO valid parse possible."};
}

void print_AST(const std::unique_ptr<ASTNode>& root, const size_t depth = 0, std::ostream& os = std::cout) {
    if (!root) return;

    if (depth > 0) {
        for(int i = 1; i < depth; i++) os << ' ';
        os << "↪";
    }
    os << root->node_type << '\n';
    for (const auto& child : root->children) print_AST(child, depth + 2, os);
}
