#ifndef XERLANG_PRINTER_H
#define XERLANG_PRINTER_H

#include "../parser/ast.h"
#include "../util/types.h"

struct Printer : public Visitor {
    void visit(struct ArgsNode&) override;
    void visit(struct DeclarationsNode&) override;
    void visit(struct ForPrologueNode&) override;
    void visit(struct ProgramNode&) override;
    void visit(struct StructDefNode&) override;
    void visit(struct ProcedureNode&) override;
    void visit(struct MainNode&) override;
    void visit(struct BlockNode&) override;
    void visit(struct DeclarationNode&) override;
    void visit(struct VarInitNode&) override;
    void visit(struct IfNode&) override;
    void visit(struct DeleteNode&) override;
    void visit(struct PrintNode&) override;
    void visit(struct ReturnNode&) override;
    void visit(struct WhileNode&) override;
    void visit(struct AssignmentNode&) override;
    void visit(struct ForNode&) override;
    void visit(struct BreakNode&) override;
    void visit(struct NumNode&) override;
    void visit(struct CharNode&) override;
    void visit(struct TrueNode&) override;
    void visit(struct FalseNode&) override;
    void visit(struct IDNode&) override;
    void visit(struct NilNode&) override;
    void visit(struct BinaryExprNode&) override;
    void visit(struct MemberAccessExprNode&) override;
    void visit(struct UnaryExprNode&) override;
    void visit(struct AllocNode&) override;
    void visit(struct FunctionCallNode&) override;
    void visit(struct ReadCallNode&) override;
};

struct TypeChecker;

#endif // XERLANG_PRINTER_H
