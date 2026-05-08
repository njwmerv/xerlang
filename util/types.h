#ifndef TYPES_H
#define TYPES_H

#include <cstddef>
#include <ostream>
#include <vector>
#include <array>

#define MAX_RHS_LEN 11

namespace Scanner {
    enum ScannerDFAState {
        // ACCEPTINGs
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT, VOID,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-ACCEPTINGs
        START, APOS, APOSLASH, NOT_CHARLIT,
        ERROR, NUM_STATES,
    };
}

std::ostream& operator<<(std::ostream& os, Scanner::ScannerDFAState state);

namespace Parser {
    enum ParserSymbol {
        // TERMINATING
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT, VOID,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-TERMINATING
        BoF, EoF, start,
        procedures, procedure, main,
        type, star,
        params, paramlist, dcls, dcl,
        statements, statement, ifs, forprologue, forepilogue,
        expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11, expr12, expr13, expr14,
        args, structdef,
        DOLLAR, NUM_SYMBOLS,
    };
}

std::ostream& operator<<(std::ostream& os, Parser::ParserSymbol state);

typedef struct {
    std::string lexeme;
    Parser::ParserSymbol type;
    size_t line_num;
    size_t col_num;
} Token;

struct Production {
    Parser::ParserSymbol LHS;
    size_t len;
    std::array<Parser::ParserSymbol, MAX_RHS_LEN> RHS;
};

struct Visitor; // Forward declaration

struct ArgsNode;
struct DeclarationsNode;
struct TypeNode;
struct StarNode;
struct ForPrologueNode;
struct ProgramNode;
struct StructDefNode;
struct ProcedureNode;
    struct MainNode;
struct BlockNode;
struct StatementNode;
    struct DeclarationNode;
    struct VarInitNode;
    struct IfNode;
    struct DeleteNode;
    struct PrintNode;
    struct ReturnNode;
    struct WhileNode;
    struct AssignmentNode;
    struct ForNode;
    struct BreakNode;
    struct ExprNode;
        struct NumNode;
        struct CharNode;
        struct TrueNode;
        struct FalseNode;
        struct IDNode;
        struct NilNode;
        struct BinaryExprNode;
        struct MemberAccessExprNode;
        struct UnaryExprNode;
        struct AllocNode;
        struct FunctionCallNode;
            struct ReadCallNode;

struct ASTNode {
    ASTNode* parent = nullptr;
    const Parser::ParserSymbol node_type;

    ASTNode() : parent{nullptr}, node_type{Parser::ParserSymbol::DOLLAR} {}
    ASTNode(Parser::ParserSymbol type) : parent{nullptr}, node_type{type} {}

    virtual ~ASTNode() = default;

    virtual void accept(struct Visitor& v) = 0;
};

// Analysis/Code Generation

struct Visitor {
    virtual ~Visitor() = default;
    virtual void visit(struct ArgsNode&) = 0;
    virtual void visit(struct DeclarationsNode&) = 0;
    virtual void visit(struct ForPrologueNode&) = 0;
    virtual void visit(struct ProgramNode&) = 0;
    virtual void visit(struct StructDefNode&) = 0;
    virtual void visit(struct ProcedureNode&) = 0;
    virtual void visit(struct MainNode&) = 0;
    virtual void visit(struct BlockNode&) = 0;
    virtual void visit(struct DeclarationNode&) = 0;
    virtual void visit(struct VarInitNode&) = 0;
    virtual void visit(struct IfNode&) = 0;
    virtual void visit(struct DeleteNode&) = 0;
    virtual void visit(struct PrintNode&) = 0;
    virtual void visit(struct ReturnNode&) = 0;
    virtual void visit(struct WhileNode&) = 0;
    virtual void visit(struct AssignmentNode&) = 0;
    virtual void visit(struct ForNode&) = 0;
    virtual void visit(struct BreakNode&) = 0;
    virtual void visit(struct NumNode&) = 0;
    virtual void visit(struct CharNode&) = 0;
    virtual void visit(struct TrueNode&) = 0;
    virtual void visit(struct FalseNode&) = 0;
    virtual void visit(struct IDNode&) = 0;
    virtual void visit(struct NilNode&) = 0;
    virtual void visit(struct BinaryExprNode&) = 0;
    virtual void visit(struct MemberAccessExprNode&) = 0;
    virtual void visit(struct UnaryExprNode&) = 0;
    virtual void visit(struct AllocNode&) = 0;
    virtual void visit(struct FunctionCallNode&) = 0;
    virtual void visit(struct ReadCallNode&) = 0;
};

struct SymbolTableEntry {

};

#endif //TYPES_H
