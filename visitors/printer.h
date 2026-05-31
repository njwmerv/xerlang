#ifndef XERLANG_PRINTER_H
#define XERLANG_PRINTER_H

#include "../parser/ast.h"
#include "../util/types.h"

struct Printer : public Visitor {
    void visit(struct ArgsNode&, std::ostream&) override;
    void visit(struct DeclarationsNode&, std::ostream&) override;
    void visit(struct ForPrologueNode&, std::ostream&) override;
    void visit(struct ProgramNode&, std::ostream&) override;
    void visit(struct StructDefNode&, std::ostream&) override;
    void visit(struct ProcedureNode&, std::ostream&) override;
    void visit(struct MainNode&, std::ostream&) override;
    void visit(struct BlockNode&, std::ostream&) override;
    void visit(struct DeclarationNode&, std::ostream&) override;
    void visit(struct VarInitNode&, std::ostream&) override;
    void visit(struct IfNode&, std::ostream&) override;
    void visit(struct DeleteNode&, std::ostream&) override;
    void visit(struct PrintNode&, std::ostream&) override;
    void visit(struct ReturnNode&, std::ostream&) override;
    void visit(struct WhileNode&, std::ostream&) override;
    void visit(struct AssignmentNode&, std::ostream&) override;
    void visit(struct ForNode&, std::ostream&) override;
    void visit(struct BreakNode&, std::ostream&) override;
    void visit(struct NumNode&, std::ostream&) override;
    void visit(struct CharNode&, std::ostream&) override;
    void visit(struct TrueNode&, std::ostream&) override;
    void visit(struct FalseNode&, std::ostream&) override;
    void visit(struct IDNode&, std::ostream&) override;
    void visit(struct NilNode&, std::ostream&) override;
    void visit(struct BinaryExprNode&, std::ostream&) override;
    void visit(struct MemberAccessExprNode&, std::ostream&) override;
    void visit(struct UnaryExprNode&, std::ostream&) override;
    void visit(struct AllocNode&, std::ostream&) override;
    void visit(struct FunctionCallNode&, std::ostream&) override;
    void visit(struct ReadCallNode&, std::ostream&) override;
};

struct TypeChecker;

#endif // XERLANG_PRINTER_H
