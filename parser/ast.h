#ifndef XERLANG_AST_H
#define XERLANG_AST_H

#include <unordered_map>
#include <memory>
#include "../util/types.h"

struct StatementNode : public ASTNode {};

// Declarations

struct DeclarationNode : public StatementNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::dcl;
    std::string type;
    std::string id;
};

struct DeclarationsNode : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::dcls;
    std::vector<std::unique_ptr<DeclarationNode>> declarations;
};

struct StructDefNodes : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::structdef;
    std::vector<std::unique_ptr<DeclarationNode>> fields;
};

// Structural

struct BlockNode : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::statements;
    std::vector<std::unique_ptr<StatementNode>> statements;
};

struct ProcedureNode : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::procedure;
    std::string id;
    std::unordered_map<std::string, SymbolTableEntry> symbol_table;
    std::unique_ptr<DeclarationsNode> params;
    std::unique_ptr<BlockNode> block;
    std::string return_type;
};

struct MainNode : public ProcedureNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::MAIN;
};

struct ProgramNode : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::start;
    std::vector<std::unique_ptr<StructDefNodes>> struct_defs;
    std::vector<std::unique_ptr<DeclarationsNode>> global_vars;
    std::vector<std::unique_ptr<ProcedureNode>> procedures;
};

// Operations

struct ExprNode : public ASTNode {
    std::string type;
};

struct NumNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::NUM;
    const int val;
    NumNode(const std::string& lexeme);
};

struct CharNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::CHARLIT;
    const char val;
    CharNode(const std::string& lexeme);
};

struct TrueNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::TRUE;
    const bool val = true;
    TrueNode();
};

struct FalseNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::FALSE;
    const bool val = false;
    FalseNode();
};

struct IDNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::ID;
    const std::string name;
    IDNode(std::string lexeme);
};

struct NilNode : public ExprNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::NIL;
    NilNode();
};

struct BinaryExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> LHS;
    std::unique_ptr<ExprNode> RHS;
    BinaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r);
};

struct MemberAccessExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> arg;
    std::unique_ptr<IDNode> id;
    MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<IDNode> r);
};

struct UnaryExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> arg;
    UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg);
};

// Args

struct ArgsNode : public ASTNode {
    static const Parser::ParserSymbol node_type = Parser::ParserSymbol::args;
    std::vector<std::unique_ptr<ExprNode>> args;
};

// Statements

struct VarInitNode : public StatementNode {
    std::unique_ptr<DeclarationNode> dcl;
    std::unique_ptr<ExprNode> val;
    VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val);
};

struct IfNode : public StatementNode {

};

struct WhileNode : public StatementNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> statements;
    WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements);
};

#endif // XERLANG_AST_H
