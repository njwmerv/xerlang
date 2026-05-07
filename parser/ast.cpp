#include "ast.h"
#include <memory>
#include <utility>

// Structural


StructDefNode::StructDefNode(std::string id, std::unique_ptr<DeclarationsNode> dcls) : id{std::move(id)}, fields{std::move(dcls)} {}

ProcedureNode::ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params, std::unique_ptr<BlockNode> block)
    : id{std::move(id)}, symbol_table{}, params{std::move(params)}, block{std::move(block)}, return_type{std::move(return_type)} {}

MainNode::MainNode(std::unique_ptr<BlockNode> bl) : ProcedureNode{"main", "int", nullptr, std::move(bl)} {}

// Operations

ExprNode::ExprNode() = default;

ExprNode::ExprNode(std::string type) : type{std::move(type)} {}

BinaryExprNode::BinaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r) : op{op}, LHS{std::move(l)}, RHS{std::move(r)} {}

MemberAccessExprNode::MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg, std::string id) : op{op}, arg{std::move(arg)}, id{std::move(id)} {}

UnaryExprNode::UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg) : op{op}, arg{std::move(arg)} {}

NumNode::NumNode(const std::string& lexeme) : val{std::stoi(lexeme)} { type = "int"; }

CharNode::CharNode(const std::string& lexeme) : val{(lexeme.size() == 3) ? lexeme.at(1) : lexeme.at(2)} { type = "char"; }

TrueNode::TrueNode() : ExprNode{"bool"}, val{true} {}

FalseNode::FalseNode() : ExprNode{"bool"}, val{false} {}

IDNode::IDNode(std::string lexeme) : ExprNode{}, name{std::move(lexeme)} {}

NilNode::NilNode() : ExprNode{"*"} {}

FunctionCallNode::FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args) : id{std::move(id)}, args{std::move(args)} {}

ReadCallNode::ReadCallNode() : FunctionCallNode{"read", nullptr} {}

AllocNode::AllocNode(std::string type, int size) : ptr_type{std::move(type)}, size{size} {}

// Arguments

TypeNode::TypeNode(std::string lexeme) : lexeme{std::move(lexeme)} {}

// Statements

VarInitNode::VarInitNode(std::unique_ptr<DeclarationNode> dcl) : dcl{std::move(dcl)}, val{nullptr} {}
VarInitNode::VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val) : dcl{std::move(dcl)}, val{std::move(val)} {}

ForNode::ForNode(std::unique_ptr<ForPrologueNode> pro, std::unique_ptr<ExprNode> cond, std::unique_ptr<StatementNode> asst, std::unique_ptr<BlockNode> block)
    : prologue{std::move(pro)}, cond{std::move(cond)}, epilogue{std::move(asst)}, block{std::move(block)} {}

ForPrologueNode::ForPrologueNode(std::unique_ptr<VarInitNode> init) : init{std::move(init)}, asst{nullptr} {}
ForPrologueNode::ForPrologueNode(std::unique_ptr<AssignmentNode> asst) : init{nullptr}, asst{std::move(asst)} {}

DeleteNode::DeleteNode(std::unique_ptr<ExprNode> ptr) : ptr{std::move(ptr)} {}

PrintNode::PrintNode(std::unique_ptr<ArgsNode> args) : args{std::move(args)} {}

ReturnNode::ReturnNode(std::unique_ptr<ExprNode> expr) : StatementNode{}, expr{std::move(expr)} {}

WhileNode::WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements) : condition{std::move(condition)}, statements{std::move(statements)} {}


// Assignment

AssignmentNode::AssignmentNode(std::unique_ptr<ExprNode> LHS, std::unique_ptr<ExprNode> RHS) : LHS{std::move(LHS)}, RHS{std::move(RHS)} {}

DeclarationNode::DeclarationNode(std::string type, std::string id) : type{std::move(type)}, id{std::move(id)} {}

ProgramNode::ProgramNode() : ASTNode{}, struct_defs{}, global_vars{}, procedures{}, main{nullptr} {}

ArgsNode::ArgsNode() : ASTNode{}, args{} {}

StarNode::StarNode() : ASTNode{}, count{0} {}

IfNode::IfNode() : StatementNode{}, clauses{} {}

BreakNode::BreakNode() : StatementNode{} {}

DeclarationsNode::DeclarationsNode() : ASTNode{}, declarations{} {}
