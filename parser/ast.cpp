#include "ast.h"
#include <memory>
#include <utility>

//// Base Classes

ArgsNode::ArgsNode() : ASTNode{Parser::ParserSymbol::args}, args{} {}

DeclarationsNode::DeclarationsNode() : ASTNode{Parser::ParserSymbol::dcls}, declarations{} {}

TypeNode::TypeNode(std::string lexeme) : ASTNode{Parser::ParserSymbol::type}, lexeme{std::move(lexeme)} {}

StarNode::StarNode() : ASTNode{Parser::ParserSymbol::AT}, count{0} {}

ForPrologueNode::ForPrologueNode(std::unique_ptr<VarInitNode> init)
    : ASTNode{Parser::ParserSymbol::forprologue}, init{std::move(init)}, asst{nullptr} {}
ForPrologueNode::ForPrologueNode(std::unique_ptr<AssignmentNode> asst)
    : ASTNode{Parser::ParserSymbol::forprologue}, init{nullptr}, asst{std::move(asst)} {}

StructDefNode::StructDefNode(std::string id)
    : ASTNode{Parser::ParserSymbol::structdef}, id{std::move(id)}, fields{} {}

ProcedureNode::ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params,
                             std::unique_ptr<BlockNode> block)
    : ASTNode{Parser::ParserSymbol::procedure}, id{std::move(id)}, symbol_table{}, params{std::move(params)},
      block{std::move(block)}, return_type{std::move(return_type)} {}
ProcedureNode::ProcedureNode(std::string id, std::string return_type, std::unique_ptr<DeclarationsNode> params,
                             std::unique_ptr<BlockNode> block, Parser::ParserSymbol node_type)
    : ASTNode{node_type}, id{std::move(id)}, symbol_table{}, params{std::move(params)},
      block{std::move(block)}, return_type{std::move(return_type)} {}

MainNode::MainNode(std::unique_ptr<BlockNode> b)
    : ProcedureNode{"main", TYPE_INT, nullptr, std::move(b), Parser::ParserSymbol::MAIN} {}

ProgramNode::ProgramNode()
    : ASTNode{Parser::ParserSymbol::start}, struct_defs{}, global_vars{}, procedures{}, main{nullptr},
      symbol_table{}, type_sizes{} {
    type_sizes.insert({TYPE_INT, SIZE_INT});
    type_sizes.insert({TYPE_CHAR, SIZE_CHAR});
    type_sizes.insert({TYPE_BOOL, SIZE_BOOL});
    type_sizes.insert({"*", SIZE_PTR});
}

int ProgramNode::get_size(const std::string& type) const {
    if (type.ends_with('*')) return type_sizes.at("*");
    return type_sizes.at(type);
}

BlockNode::BlockNode() : ASTNode{Parser::ParserSymbol::statements}, statements{} {}

//// Statements

StatementNode::StatementNode(Parser::ParserSymbol node_type) : ASTNode{node_type} {}

// Expressions

ExprNode::ExprNode() : StatementNode{Parser::ParserSymbol::expr1} {}
ExprNode::ExprNode(Parser::ParserSymbol node_type) : StatementNode{node_type} {}
ExprNode::ExprNode(std::string type, Parser::ParserSymbol node_type)
    : StatementNode{node_type}, type{std::move(type)} {}

NumNode::NumNode(const std::string& lexeme)
    : ExprNode{TYPE_INT, Parser::ParserSymbol::NUM}, val{std::stoi(lexeme)} {}

char unescape_char(const std::string& lexeme) {
    if (lexeme.empty()) throw std::invalid_argument("Empty lexeme");
    else if (lexeme.length() == 3) return lexeme[1];
    else if (lexeme.length() == 4) {
        switch (lexeme[2]) {
            case 'n':  return '\n';
            case 't':  return '\t';
            case 'r':  return '\r';
            case '0':  return '\0';
            case 'b':  return '\b';
            case 'f':  return '\f';
            case 'v':  return '\v';
            case '\\': return '\\';
            case '\'': return '\'';
            case '\"': return '\"';
            default: return lexeme[1];
        }
    }
    throw std::invalid_argument("Invalid CHARLIT lexeme");
}

CharNode::CharNode(const std::string& lexeme)
    : ExprNode{TYPE_CHAR, Parser::ParserSymbol::CHARLIT},
      val{unescape_char(lexeme)} {}

TrueNode::TrueNode() : ExprNode{TYPE_BOOL, Parser::ParserSymbol::TRUE}, val{true} {}

FalseNode::FalseNode() : ExprNode{TYPE_BOOL, Parser::ParserSymbol::FALSE}, val{false} {}

IDNode::IDNode(std::string lexeme)
    : ExprNode{Parser::ParserSymbol::ID}, name{std::move(lexeme)}, offset{1} {}

NilNode::NilNode() : ExprNode{"*", Parser::ParserSymbol::NIL} {}

BinaryExprNode::BinaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r)
    : ExprNode{op}, op{op}, LHS{std::move(l)}, RHS{std::move(r)} {}

MemberAccessExprNode::MemberAccessExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg, std::string id)\
    : ExprNode{op}, op{op}, arg{std::move(arg)}, id{std::move(id)} {}

UnaryExprNode::UnaryExprNode(Parser::ParserSymbol op, std::unique_ptr<ExprNode> arg)
    : ExprNode{op}, op{op}, arg{std::move(arg)} {}

AllocNode::AllocNode(std::string type, int size)
    : ExprNode{Parser::ParserSymbol::NEW}, ptr_type{std::move(type)}, size{size} {}

FunctionCallNode::FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args)
    : ExprNode{Parser::ParserSymbol::paramlist}, id{std::move(id)}, args{std::move(args)} {}
FunctionCallNode::FunctionCallNode(std::string id, std::unique_ptr<ArgsNode> args, Parser::ParserSymbol node_type)
    : ExprNode{node_type}, id{std::move(id)}, args{std::move(args)} {}

ReadCallNode::ReadCallNode() : FunctionCallNode{"read", nullptr, Parser::ParserSymbol::READ} {}

/////////

// Statements

DeclarationNode::DeclarationNode(std::string id, std::string type)
    : StatementNode{Parser::ParserSymbol::dcl}, id{std::move(id)}, type{std::move(type)}, frame_offset{0} {}

VarInitNode::VarInitNode(std::unique_ptr<DeclarationNode> dcl)
    : StatementNode{Parser::ParserSymbol::expr2}, dcl{std::move(dcl)}, val{nullptr} {}
VarInitNode::VarInitNode(std::unique_ptr<DeclarationNode> dcl, std::unique_ptr<ExprNode> val)
    : StatementNode{Parser::ParserSymbol::expr2}, dcl{std::move(dcl)}, val{std::move(val)} {}

IfNode::IfNode() : StatementNode{Parser::ParserSymbol::IF}, clauses{} {}

DeleteNode::DeleteNode(std::unique_ptr<ExprNode> ptr)
    : StatementNode{Parser::ParserSymbol::DELETE}, ptr{std::move(ptr)} {}

PrintNode::PrintNode(std::unique_ptr<ArgsNode> args)
    : StatementNode{Parser::ParserSymbol::PRINT}, args{std::move(args)} {}

ReturnNode::ReturnNode(std::unique_ptr<ExprNode> expr)
    : StatementNode{Parser::ParserSymbol::RETURN}, expr{std::move(expr)} {}

WhileNode::WhileNode(std::unique_ptr<ExprNode> condition, std::unique_ptr<BlockNode> statements)
    : StatementNode{Parser::ParserSymbol::WHILE}, condition{std::move(condition)}, statements{std::move(statements)} {}

AssignmentNode::AssignmentNode(std::unique_ptr<ExprNode> LHS, std::unique_ptr<ExprNode> RHS)
    : StatementNode{Parser::ParserSymbol::BECOMES}, LHS{std::move(LHS)}, RHS{std::move(RHS)} {}

ForNode::ForNode(std::unique_ptr<ForPrologueNode> pro, std::unique_ptr<ExprNode> cond,
                 std::unique_ptr<StatementNode> asst, std::unique_ptr<BlockNode> block)
    : StatementNode{Parser::ParserSymbol::FOR}, prologue{std::move(pro)}, cond{std::move(cond)},
      epilogue{std::move(asst)}, block{std::move(block)} {}

BreakNode::BreakNode() : StatementNode{Parser::ParserSymbol::BREAK} {}

//// Visitor

std::any ArgsNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any DeclarationsNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ForPrologueNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ProgramNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any StructDefNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ProcedureNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any MainNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any BlockNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any DeclarationNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any VarInitNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any IfNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any DeleteNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any PrintNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ReturnNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any WhileNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any AssignmentNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ForNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any BreakNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any NumNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any CharNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any TrueNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any FalseNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any IDNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any NilNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any BinaryExprNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any MemberAccessExprNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any UnaryExprNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any AllocNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any FunctionCallNode::accept(struct Visitor& v) { return v.visit(*this); }
std::any ReadCallNode::accept(struct Visitor& v) { return v.visit(*this); }