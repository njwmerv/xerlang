#ifndef XERLANG_AST_H
#define XERLANG_AST_H

#include <unordered_map>
#include <memory>
#include <map>
#include "../util/types.h"
#include "symbol_table.h"

//// Base Classes

struct ArgsNode : public ASTNode {
    std::vector<std::unique_ptr<ExprNode>> args;
    ArgsNode();
    std::any accept(Visitor& v) override;
};

struct DeclarationsNode : public ASTNode {
    std::vector<std::unique_ptr<DeclarationNode>> declarations;
    DeclarationsNode();
    std::any accept(Visitor& v) override;
};

struct TypeNode : public ASTNode {
    const std::string lexeme;
    TypeNode(std::string lexeme);
    std::any accept(Visitor& v) override { return std::any{}; }
};

struct StarNode : public ASTNode {
    size_t count = 0;
    StarNode();
    std::any accept(Visitor& v) override { return std::any{}; }
};

struct ForPrologueNode : public ASTNode {
    std::unique_ptr<VarInitNode> init;
    std::unique_ptr<AssignmentNode> asst;
    ForPrologueNode(std::unique_ptr<VarInitNode> init);
    ForPrologueNode(std::unique_ptr<AssignmentNode> asst);
    std::any accept(Visitor& v) override;
};

struct StructDefNode : public ASTNode {
    const std::string id;
    std::map<std::string, std::unique_ptr<DeclarationNode>> fields;
    StructDefNode(std::string id);
    std::any accept(Visitor& v) override;
};

struct ProcedureNode : public ASTNode {
    std::string id;
    SymbolTable symbol_table;
    std::unique_ptr<DeclarationsNode> params;
    std::unique_ptr<BlockNode> block;
    std::string return_type;
    ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block);
    ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block, Parser::ParserSymbol node_type);
    std::any accept(Visitor& v) override;
};

struct MainNode : public ProcedureNode {
    MainNode(std::unique_ptr<BlockNode> b);
    std::any accept(Visitor& v) override;
};

struct ProgramNode : public ASTNode {
    std::unordered_map<std::string, std::unique_ptr<StructDefNode>> struct_defs;
    std::vector<std::unique_ptr<VarInitNode>> global_vars;
    std::unordered_map<std::string, std::unique_ptr<ProcedureNode>> procedures;
    std::unique_ptr<MainNode> main;
    std::unordered_map<std::string, SymbolTableEntry> symbol_table;
    std::unordered_map<std::string, int> type_sizes;
    ProgramNode();
    std::any accept(Visitor& v) override;

    int get_size(const std::string& type) const;
};

struct BlockNode : public ASTNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
    BlockNode();
    std::any accept(Visitor& v) override;
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
    std::any accept(Visitor& v) override;
};

struct CharNode : public ExprNode {
    const char val;
    CharNode(const std::string& lexeme);
    std::any accept(Visitor& v) override;
};

struct TrueNode : public ExprNode {
    const bool val = true;
    TrueNode();
    std::any accept(Visitor& v) override;
};

struct FalseNode : public ExprNode {
    const bool val = false;
    FalseNode();
    std::any accept(Visitor& v) override;
};

struct IDNode : public ExprNode {
    const std::string name;
    int offset = 1;
    IDNode(std::string lexeme);
    std::any accept(Visitor& v) override;
};

struct NilNode : public ExprNode {
    NilNode();
    std::any accept(Visitor& v) override;
};

struct BinaryExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> LHS;
    std::unique_ptr<ExprNode> RHS;
    BinaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r);
    std::any accept(Visitor& v) override;
};

struct MemberAccessExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> arg;
    const std::string id;
    MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg, std::string id);
    std::any accept(Visitor& v) override;
};

struct UnaryExprNode : public ExprNode {
    Parser::ParserSymbol op;
    std::unique_ptr<ExprNode> arg;
    UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg);
    std::any accept(Visitor& v) override;
};

struct AllocNode : public ExprNode {
    const std::string ptr_type;
    const int size;
    AllocNode(std::string type, int size);
    std::any accept(Visitor& v) override;
};

struct FunctionCallNode : public ExprNode {
    const std::string id;
    std::unique_ptr<ArgsNode> args;
    FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args);
    FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args, Parser::ParserSymbol node_type);
    std::any accept(Visitor& v) override;
};

struct ReadCallNode : public FunctionCallNode {
    ReadCallNode();
    std::any accept(Visitor& v) override;
};

// Statements

struct DeclarationNode : public StatementNode {
    std::string id;
    std::string type;
    int frame_offset = 0;
    DeclarationNode(std::string id, std::string type);
    std::any accept(Visitor& v) override;
};

struct VarInitNode : public StatementNode {
    std::unique_ptr<DeclarationNode> dcl;
    std::unique_ptr<ExprNode> val;
    VarInitNode(std::unique_ptr<DeclarationNode> dcl);
    VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val);
    std::any accept(Visitor& v) override;
};

struct IfNode : public StatementNode {
    struct IfClause {
        std::unique_ptr<ExprNode> cond;
        std::unique_ptr<BlockNode> block;
    };

    std::vector<IfClause> clauses;
    IfNode();
    std::any accept(Visitor& v) override;
};

struct DeleteNode : public StatementNode {
    std::unique_ptr<ExprNode> ptr;
    DeleteNode(std::unique_ptr<ExprNode> ptr);
    std::any accept(Visitor& v) override;
};

struct PrintNode : public StatementNode {
    std::unique_ptr<ArgsNode> args;
    PrintNode(std::unique_ptr<ArgsNode> args);
    std::any accept(Visitor& v) override;
};

struct ReturnNode : public StatementNode {
    std::unique_ptr<ExprNode> expr;
    ReturnNode(std::unique_ptr<ExprNode> expr);
    std::any accept(Visitor& v) override;
};

struct WhileNode : public StatementNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<BlockNode> statements;
    WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements);
    std::any accept(Visitor& v) override;
};

struct AssignmentNode : public StatementNode {
    std::unique_ptr<ExprNode> LHS;
    std::unique_ptr<ExprNode> RHS;
    AssignmentNode(std::unique_ptr<ExprNode> LHS, std::unique_ptr<ExprNode> RHS);
    std::any accept(Visitor& v) override;
};

struct ForNode : public StatementNode {
    std::unique_ptr<ForPrologueNode> prologue;
    std::unique_ptr<ExprNode> cond;
    std::unique_ptr<StatementNode> epilogue;
    std::unique_ptr<BlockNode> block;
    ForNode(std::unique_ptr<ForPrologueNode> pro, std::unique_ptr<ExprNode> cond, std::unique_ptr<StatementNode> asst, std::unique_ptr<BlockNode> block);
    std::any accept(Visitor& v) override;
};

struct BreakNode : public StatementNode {
    ASTNode* loop_target = nullptr;
    BreakNode();
    std::any accept(Visitor& v) override;
};

#endif // XERLANG_AST_H
