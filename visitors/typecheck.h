#ifndef XERLANG_TYPECHECK_H
#define XERLANG_TYPECHECK_H

#include "../parser/ast.h"
#include "../util/types.h"

struct TypeChecker : public Visitor {
    std::any visit(struct ArgsNode&) override;
    std::any visit(struct DeclarationsNode&) override;
    std::any visit(struct ForPrologueNode&) override;
    std::any visit(struct ProgramNode&) override;
    std::any visit(struct StructDefNode&) override;
    std::any visit(struct ProcedureNode&) override;
    std::any visit(struct MainNode&) override;
    std::any visit(struct BlockNode&) override;
    std::any visit(struct DeclarationNode&) override;
    std::any visit(struct VarInitNode&) override;
    std::any visit(struct IfNode&) override;
    std::any visit(struct DeleteNode&) override;
    std::any visit(struct PrintNode&) override;
    std::any visit(struct ReturnNode&) override;
    std::any visit(struct WhileNode&) override;
    std::any visit(struct AssignmentNode&) override;
    std::any visit(struct ForNode&) override;
    std::any visit(struct BreakNode&) override;
    std::any visit(struct NumNode&) override;
    std::any visit(struct CharNode&) override;
    std::any visit(struct TrueNode&) override;
    std::any visit(struct FalseNode&) override;
    std::any visit(struct IDNode&) override;
    std::any visit(struct NilNode&) override;
    std::any visit(struct BinaryExprNode&) override;
    std::any visit(struct MemberAccessExprNode&) override;
    std::any visit(struct UnaryExprNode&) override;
    std::any visit(struct AllocNode&) override;
    std::any visit(struct FunctionCallNode&) override;
    std::any visit(struct ReadCallNode&) override;
};

#endif // XERLANG_TYPECHECK_H
