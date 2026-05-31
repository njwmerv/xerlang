#ifndef XERLANG_TYPECHECK_H
#define XERLANG_TYPECHECK_H

#include "../parser/ast.h"
#include "../util/types.h"

struct TypeChecker : public Visitor {
    void visit(struct ArgsNode&, std::ostream& os) override;
    void visit(struct DeclarationsNode&, std::ostream& os) override;
    void visit(struct ForPrologueNode&, std::ostream& os) override;
    void visit(struct ProgramNode&, std::ostream& os) override;
    void visit(struct StructDefNode&, std::ostream& os) override;
    void visit(struct ProcedureNode&, std::ostream& os) override;
    void visit(struct MainNode&, std::ostream& os) override;
    void visit(struct BlockNode&, std::ostream& os) override;
    void visit(struct DeclarationNode&, std::ostream& os) override;
    void visit(struct VarInitNode&, std::ostream& os) override;
    void visit(struct IfNode&, std::ostream& os) override;
    void visit(struct DeleteNode&, std::ostream& os) override;
    void visit(struct PrintNode&, std::ostream& os) override;
    void visit(struct ReturnNode&, std::ostream& os) override;
    void visit(struct WhileNode&, std::ostream& os) override;
    void visit(struct AssignmentNode&, std::ostream& os) override;
    void visit(struct ForNode&, std::ostream& os) override;
    void visit(struct BreakNode&, std::ostream& os) override;
    void visit(struct NumNode&, std::ostream& os) override;
    void visit(struct CharNode&, std::ostream& os) override;
    void visit(struct TrueNode&, std::ostream& os) override;
    void visit(struct FalseNode&, std::ostream& os) override;
    void visit(struct IDNode&, std::ostream& os) override;
    void visit(struct NilNode&, std::ostream& os) override;
    void visit(struct BinaryExprNode&, std::ostream& os) override;
    void visit(struct MemberAccessExprNode&, std::ostream& os) override;
    void visit(struct UnaryExprNode&, std::ostream& os) override;
    void visit(struct AllocNode&, std::ostream& os) override;
    void visit(struct FunctionCallNode&, std::ostream& os) override;
    void visit(struct ReadCallNode&, std::ostream& os) override;
};

#endif // XERLANG_TYPECHECK_H
