#ifndef XERLANG_AST_H
#define XERLANG_AST_H

#include <unordered_map>
#include <memory>
#include "../util/types.h"

struct StatementNode;
    struct VarInitNode;
    struct IfNode;
    struct DeleteNode;
    struct PrintNode;
    struct ReturnNode;
    struct WhileNode;
    struct AssignmentNode;
    struct ForNode;
    struct BreakNode;
struct DeclarationNode;
struct DeclarationsNode;
struct StructDefNode;
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
struct ArgsNode;
struct TypeNode;
struct StarNode;
struct BlockNode;
struct ForPrologueNode;
struct ProcedureNode;
    struct MainNode;
struct ProgramNode;

struct StatementNode : public ASTNode {};

// Declarations

struct DeclarationNode : public StatementNode {
    std::string type;
    std::string id;
    DeclarationNode(std::string type, std::string id);
};

struct DeclarationsNode : public ASTNode {
    std::vector<std::unique_ptr<DeclarationNode>> declarations;
    DeclarationsNode();
};

struct StructDefNode : public ASTNode {
    const std::string id;
    std::unique_ptr<DeclarationsNode> fields;
    StructDefNode(std::string id, std::unique_ptr<DeclarationsNode> dcls);
};


// Arithmetic

struct ExprNode : public StatementNode {
    std::string type;
    ExprNode();
    ExprNode(std::string type);
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

// Args

struct ArgsNode : public ASTNode {
    std::vector<std::unique_ptr<ExprNode>> args;
    ArgsNode();
};

struct FunctionCallNode : public ExprNode {
    const std::string id;
    std::unique_ptr<ArgsNode> args;
    FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args);
};

struct ReadCallNode : public FunctionCallNode {
    ReadCallNode();
};

struct TypeNode : public ASTNode {
    const std::string lexeme;
    TypeNode(std::string lexeme);
};

struct StarNode : public ASTNode {
    size_t count = 0;
    StarNode();
};

// Statements

struct VarInitNode : public StatementNode {
    std::unique_ptr<DeclarationNode> dcl;
    std::unique_ptr<ExprNode> val;
    VarInitNode(std::unique_ptr<DeclarationNode> dcl);
    VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val);
};

struct BlockNode : public ASTNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
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

// Assignment

struct AssignmentNode : public StatementNode {
    std::unique_ptr<ExprNode> LHS;
    std::unique_ptr<ExprNode> RHS;
    AssignmentNode(std::unique_ptr<ExprNode> LHS, std::unique_ptr<ExprNode> RHS);
};

struct ForPrologueNode : public ASTNode {
    std::unique_ptr<VarInitNode> init;
    std::unique_ptr<AssignmentNode> asst;
    ForPrologueNode(std::unique_ptr<VarInitNode> init);
    ForPrologueNode(std::unique_ptr<AssignmentNode> asst);
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

// Structural

struct ProcedureNode : public ASTNode {
    std::string id;
    std::unordered_map<std::string, SymbolTableEntry> symbol_table;
    std::unique_ptr<DeclarationsNode> params;
    std::unique_ptr<BlockNode> block;
    std::string return_type;
    ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block);
};

struct MainNode : public ProcedureNode {
    MainNode(std::unique_ptr<BlockNode> block);
};

struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<StructDefNode>> struct_defs;
    std::vector<std::unique_ptr<VarInitNode>> global_vars;
    std::vector<std::unique_ptr<ProcedureNode>> procedures;
    std::unique_ptr<MainNode> main;
    ProgramNode();
};

#endif // XERLANG_AST_H
