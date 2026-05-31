#include "printer.h"
#include <iostream>
#include <sstream>
#include "visitor_helper.h"

#define INDENT 4

// Helpers

size_t depth(const struct ASTNode& ast) {
    size_t depth = 0;
    const ASTNode* a = &ast;
    while ((a = a->parent)) depth++;
    return INDENT * depth;
}

void print_indent(size_t indent, const std::string& message, std::ostream& os) {
    for (size_t i = 1; i < indent; i++) os << ' ';
    os << message;
}

inline void print_STE(size_t indent, const SymbolTableEntry& STE, std::ostream& os) {
    print_indent(indent + INDENT, "> " + STE.id + " : " + STE.type + " : " + std::to_string(STE.frame_offset) + '\n', os);
}

// Implementation

void Printer::visit(struct ArgsNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Args\n", os);
    for (auto& arg : a.args) arg->accept(*this, os);
}
void Printer::visit(struct DeclarationsNode& a, std::ostream& os) {
    if (a.declarations.empty()) return;
    print_indent(depth(a), "↪ Declarations\n", os);
    for (auto& dec : a.declarations) dec->accept(*this, os);
}
void Printer::visit(struct ForPrologueNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ For Prologue\n", os);
    if (a.init) a.init->accept(*this, os);
    if (a.asst) a.asst->accept(*this, os);
}
void Printer::visit(struct ProgramNode& a, std::ostream& os) {
    print_indent(0, "↪ Program\n", os);

    print_indent(INDENT >> 1, " > Struct Definitions\n", os);
    for (auto& [type, sd] : a.struct_defs) sd->accept(*this, os);

    print_indent(INDENT >> 1, " > Type Sizes\n", os);
    for (auto& [type, size] : a.type_sizes) {
        print_indent(INDENT, "> " + type + " : " + std::to_string(size) + " b\n", os);
    }

    print_indent(INDENT >> 1, " > Global Vars\n", os);
    for (auto& gv : a.global_vars) gv->accept(*this, os);

    print_indent(INDENT >> 1, " > Global Symbol Table\n", os);
    for (auto& [name, STE] : a.symbol_table)
        print_STE(0, STE, os);

    print_indent(INDENT >> 1, " > Procedures\n", os);
    for (auto& [id, proc] : a.procedures) proc->accept(*this, os);

    // print_indent(indent + (INDENT >> 1), "> Main\n");
    a.main->accept(*this, os);
}
void Printer::visit(struct StructDefNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Struct Definition : " + a.id + '\n', os);
    for (const auto& [field, dcl] : a.fields) {
        dcl->accept(*this, os);
    }
}
void Printer::visit(struct ProcedureNode& a, std::ostream& os) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Procedure: " + a.id + " -> " + a.return_type + '\n', os);

    if (!a.params->declarations.empty()) {
        print_indent(indent + (INDENT >> 1), "> Parameters\n", os);
        a.params->accept(*this, os);
    }

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n", os);
    for (auto& STE : a.symbol_table.see_syms()) {
        print_STE(indent, STE, os);
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n", os);
    a.block->accept(*this, os);
}
void Printer::visit(struct MainNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Main: main\n", os);

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n", os);
    for (auto& STE : a.symbol_table.see_syms()) {
        print_STE(indent, STE, os);
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n", os);
    a.block->accept(*this, os);
}
void Printer::visit(struct BlockNode& a, std::ostream& os) {
    if (a.statements.empty()) return;
    print_indent(depth(a), "↪ Block\n", os);
    for (auto& s : a.statements) s->accept(*this, os);
}
void Printer::visit(struct DeclarationNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Declaration: " + a.id + " : " + a.type + " : " + std::to_string(a.frame_offset) + '\n', os);
}
void Printer::visit(struct VarInitNode& a, std::ostream& os) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Variable Initialization\n", os);

    // print_indent(indent + (INDENT >> 1), "> Declaration\n");
    a.dcl->accept(*this, os);

    if (!a.val) return;
    // print_indent(indent + (INDENT >> 1), "> Initial Value\n");
    a.val->accept(*this, os);
}
void Printer::visit(struct IfNode& a, std::ostream& os) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ If Tree\n", os);

    auto it = a.clauses.begin();
    print_indent(indent + (INDENT >> 1), "> IF\n", os);
    // print_indent(indent + INDENT, "> Condition\n");
    it->cond->accept(*this, os);
    // print_indent(indent + INDENT, "> Statements\n");
    it->block->accept(*this, os);

    it++;
    for (; it != a.clauses.end(); it++) {
         print_indent(indent + (INDENT >> 1), "> ", os);
        if (it->cond) {
            os << "ELIF\n";
            // print_indent(indent + INDENT, "> Condition\n");
            it->cond->accept(*this, os);
        }
        else {
            os << "ELSE\n";
        }
        // print_indent(indent + INDENT, "> Statements\n");
        it->block->accept(*this, os);
    }
}
void Printer::visit(struct DeleteNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Delete\n", os);

    // print_indent(indent + (INDENT >> 1), "> Pointer\n");
    a.ptr->accept(*this, os);
}
void Printer::visit(struct PrintNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Print\n", os);

    // print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this, os);
}
void Printer::visit(struct ReturnNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Return\n", os);

    if (!a.expr) return;
    // print_indent(indent + (INDENT >> 1), "> Expr\n");
    a.expr->accept(*this, os);
}
void Printer::visit(struct WhileNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ While\n", os);

    // print_indent(indent + (INDENT >> 1), "> Condition\n");
    a.condition->accept(*this, os);

    // print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.statements->accept(*this, os);
}
void Printer::visit(struct AssignmentNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Assignment\n", os);

    print_indent(indent + (INDENT >> 1), "> LValue\n", os);
    a.LHS->accept(*this, os);

    print_indent(indent + (INDENT >> 1), "> New Value\n", os);
    a.RHS->accept(*this, os);
}
void Printer::visit(struct ForNode& a, std::ostream& os) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ For\n", os);

    // print_indent(indent + (INDENT >> 1), "> For Prologue\n");
    a.prologue->accept(*this, os);

    print_indent(indent + (INDENT >> 1), "> Condition\n", os);
    a.cond->accept(*this, os);

    print_indent(indent + (INDENT >> 1), "> For Epilogue\n", os);
    a.epilogue->accept(*this, os);

    print_indent(indent + (INDENT >> 1), "> Statements\n", os);
    a.block->accept(*this, os);
}
void Printer::visit(struct BreakNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Break\n", os);
}
void Printer::visit(struct NumNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Integer : " + std::to_string(a.val) + '\n', os);
}
void Printer::visit(struct CharNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Character : ", os);
    os << a.val << '\n';
}
void Printer::visit(struct TrueNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Boolean : TRUE\n", os);
}
void Printer::visit(struct FalseNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Boolean : FALSE\n", os);
}
void Printer::visit(struct IDNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ ID : " + a.name, os);
    if (!a.type.empty()) os << " : " << a.type;
    os << " : " << a.offset << '\n';
}
void Printer::visit(struct NilNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Pointer : NULL\n", os);
}
void Printer::visit(struct BinaryExprNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Binary Expression: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str(), os);

    // print_indent(indent + (INDENT >> 1), "> Left Side\n");
    a.LHS->accept(*this, os);

    // print_indent(indent + (INDENT >> 1), "> Right Side\n");
    a.RHS->accept(*this, os);
}
void Printer::visit(struct MemberAccessExprNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Member Access: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str(), os);

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this, os);

     print_indent(indent + (INDENT >> 1), "> Field : ", os);
     os << a.id + '\n';
}
void Printer::visit(struct UnaryExprNode& a, std::ostream& os) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Unary Expression: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str(), os);

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this, os);
}
void Printer::visit(struct AllocNode& a, std::ostream& os) {
    print_indent(depth(a), "↪ Allocation: " + a.ptr_type + " [ " + std::to_string(a.size) + " ]\n", os);
}
void Printer::visit(struct FunctionCallNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    get_parent_nodes(&a, &prog);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);

    const size_t indent = depth(a);
    print_indent(indent, "↪ Function Call: " + a.id + " -> " + PROG->procedures.at(a.id)->return_type + '\n', os);

    if (!a.args) return;
//    print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this, os);
}
void Printer::visit(struct ReadCallNode& a, std::ostream& os) {
    std::ostringstream oss;
    oss << "↪ Function Call: read -> " << TYPE_CHAR << '\n';
    print_indent(depth(a), oss.str(), os);
}
