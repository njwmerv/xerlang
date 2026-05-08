#ifndef XERLANG_AST_H
#define XERLANG_AST_H

#include <unordered_map>
#include <memory>
#include "../util/types.h"

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

//// Base Classes

struct ArgsNode : public ASTNode {
    std::vector<std::unique_ptr<ExprNode>> args;
    ArgsNode();
};

struct DeclarationsNode : public ASTNode {
    std::vector<std::unique_ptr<DeclarationNode>> declarations;
    DeclarationsNode();
};

struct TypeNode : public ASTNode {
    const std::string lexeme;
    TypeNode(std::string lexeme);
};

struct StarNode : public ASTNode {
    size_t count = 0;
    StarNode();
};

struct ForPrologueNode : public ASTNode {
    std::unique_ptr<VarInitNode> init;
    std::unique_ptr<AssignmentNode> asst;
    ForPrologueNode(std::unique_ptr<VarInitNode> init);
    ForPrologueNode(std::unique_ptr<AssignmentNode> asst);
};

struct StructDefNode : public ASTNode {
    const std::string id;
    std::unique_ptr<DeclarationsNode> fields;
    StructDefNode(std::string id, std::unique_ptr<DeclarationsNode> dcls);
};

struct ProcedureNode : public ASTNode {
    std::string id;
    std::unordered_map<std::string, SymbolTableEntry> symbol_table;
    std::unique_ptr<DeclarationsNode> params;
    std::unique_ptr<BlockNode> block;
    std::string return_type;
    ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block);
    ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block, Parser::ParserSymbol node_type);
};

struct MainNode : public ProcedureNode {
    MainNode(std::unique_ptr<BlockNode> b);
};

struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<StructDefNode>> struct_defs;
    std::vector<std::unique_ptr<VarInitNode>> global_vars;
    std::vector<std::unique_ptr<ProcedureNode>> procedures;
    std::unique_ptr<MainNode> main;
    ProgramNode();
};

struct BlockNode : public ASTNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
    BlockNode();
};

//// Statements

struct StatementNode : public ASTNode {
    StatementNode(Parser::ParserSymbol node_type);
};

// Expressions

struct ExprNode : public StatementNode {
    std::string type;
    ExprNode();
    ExprNode(Parser::ParserSymbol node_type);
    ExprNode(std::string type, Parser::ParserSymbol node_type);
};

struct NumNode : public ExprNode {
    const int val;
    NumNode(const std::string& lexeme);
};

struct CharNode : public ExprNode {
    const char val;
    CharNode(const std::string& lexeme);
};

struct TrueNode : public ExprNode {
    const bool val = true;
    TrueNode();
};

struct FalseNode : public ExprNode {
    const bool val = false;
    FalseNode();
};

struct IDNode : public ExprNode {
    const std::string name;
    IDNode(std::string lexeme);
};

struct NilNode : public ExprNode {
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
    const std::string id;
    MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg, std::string id);
};

struct UnaryExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> arg;
    UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg);
};

struct AllocNode : public ExprNode {
    const std::string ptr_type;
    const int size;
    AllocNode(std::string type, int size);
};

struct FunctionCallNode : public ExprNode {
    const std::string id;
    std::unique_ptr<ArgsNode> args;
    FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args);
    FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args, Parser::ParserSymbol node_type);
};

struct ReadCallNode : public FunctionCallNode {
    ReadCallNode();
};

// Statements

struct DeclarationNode : public StatementNode {
    std::string type;
    std::string id;
    DeclarationNode(std::string type, std::string id);
};

struct VarInitNode : public StatementNode {
    std::unique_ptr<DeclarationNode> dcl;
    std::unique_ptr<ExprNode> val;
    VarInitNode(std::unique_ptr<DeclarationNode> dcl);
    VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val);
};

struct IfNode : public StatementNode {
    struct IfClause {
        std::unique_ptr<ExprNode> cond;
        std::unique_ptr<BlockNode> block;
    };

    std::vector<IfClause> clauses;
    IfNode();
};

struct DeleteNode : public StatementNode {
    std::unique_ptr<ExprNode> ptr;
    DeleteNode(std::unique_ptr<ExprNode> ptr);
};

struct PrintNode : public StatementNode {
    std::unique_ptr<ArgsNode> args;
    PrintNode(std::unique_ptr<ArgsNode> args);
};

struct ReturnNode : public StatementNode {
    std::unique_ptr<ExprNode> expr;
    ReturnNode(std::unique_ptr<ExprNode> expr);
};

struct WhileNode : public StatementNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> statements;
    WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements);
};

struct AssignmentNode : public StatementNode {
    std::unique_ptr<ExprNode> LHS;
    std::unique_ptr<ExprNode> RHS;
    AssignmentNode(std::unique_ptr<ExprNode> LHS, std::unique_ptr<ExprNode> RHS);
};

struct ForNode : public StatementNode {
    std::unique_ptr<ForPrologueNode> prologue;
    std::unique_ptr<ExprNode> cond;
    std::unique_ptr<StatementNode> epilogue;
    std::unique_ptr<BlockNode> block;
    ForNode(std::unique_ptr<ForPrologueNode> pro, std::unique_ptr<ExprNode> cond, std::unique_ptr<StatementNode> asst, std::unique_ptr<BlockNode> block);
};

struct BreakNode : public StatementNode {
    BreakNode();
};

#endif // XERLANG_AST_H
