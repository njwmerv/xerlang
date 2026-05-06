#include "ast.h"
#include <memory>
#include <utility>

// Structural

struct ProcedureNode : public ASTNode {
    std::string id;
    std::unordered_map<std::string, SymbolTableEntry> symbol_table;
    std::string return_type;
};

struct MainNode;

struct ProgramNode : public ASTNode {

};

// Operations

BinaryExprNode::BinaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r) : op{op}, LHS{std::move(l)}, RHS{std::move(r)} {}

MemberAccessExprNode::MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg, std::unique_ptr<IDNode> id) : op{op}, arg{std::move(arg)}, id{std::move(id)} {}

UnaryExprNode::UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg) : op{op}, arg{std::move(arg)} {}

NumNode::NumNode(const std::string& lexeme) : val{std::stoi(lexeme)} { type = "int"; }

CharNode::CharNode(const std::string& lexeme) : val{(lexeme.size() == 3) ? lexeme.at(1) : lexeme.at(2)} { type = "char"; }

TrueNode::TrueNode() { type = "bool"; }

FalseNode::FalseNode() { type = "bool"; }

IDNode::IDNode(std::string lexeme) : name{std::move(lexeme)} {}

NilNode::NilNode() { type = "*"; }

// Statements

VarInitNode::VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val) : dcl{std::move(dcl)}, val{std::move(val)} {}

WhileNode::WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements) : condition{std::move(condition)}, statements{std::move(statements)} {}
