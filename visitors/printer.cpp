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

void Printer::print_indent(size_t indent, const std::string& message) {
    for (size_t i = 1; i < indent; i++) os << ' ';
    os << message;
}

void Printer::print_STE(size_t indent, const SymbolTableEntry& STE) {
    print_indent(indent + INDENT, "> " + STE.id + " : " + STE.type + " : " + std::to_string(STE.frame_offset) + '\n');
}

// Implementation

Printer::Printer(std::ostream& os) : os{os} {}

std::any Printer::visit(struct ArgsNode& a) {
    print_indent(depth(a), "↪ Args\n");
    for (auto& arg : a.args) arg->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct DeclarationsNode& a) {
    if (a.declarations.empty()) return std::any{};
    print_indent(depth(a), "↪ Declarations\n");
    for (auto& dec : a.declarations) dec->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct ForPrologueNode& a) {
    print_indent(depth(a), "↪ For Prologue\n");
    if (a.init) a.init->accept(*this);
    if (a.asst) a.asst->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct ProgramNode& a) {
    print_indent(0, "↪ Program\n");

    print_indent(INDENT >> 1, " > Struct Definitions\n");
    for (auto& [type, sd] : a.struct_defs) sd->accept(*this);

    print_indent(INDENT >> 1, " > Type Sizes\n");
    for (auto& [type, size] : a.type_sizes) {
        print_indent(INDENT, "> " + type + " : " + std::to_string(size) + " b\n");
    }

    print_indent(INDENT >> 1, " > Global Vars\n");
    for (auto& gv : a.global_vars) gv->accept(*this);

    print_indent(INDENT >> 1, " > Global Symbol Table\n");
    for (auto& [name, STE] : a.symbol_table)
        print_STE(0, STE);

    print_indent(INDENT >> 1, " > Procedures\n");
    for (auto& [id, proc] : a.procedures) proc->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Main\n");
    a.main->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct StructDefNode& a) {
    print_indent(depth(a), "↪ Struct Definition : " + a.id + '\n');
    for (const auto& [field, dcl] : a.fields) {
        dcl->accept(*this);
    }
    return std::any{};
}
std::any Printer::visit(struct ProcedureNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Procedure: " + a.id + " -> " + a.return_type + '\n');

    if (!a.params->declarations.empty()) {
        print_indent(indent + (INDENT >> 1), "> Parameters\n");
        a.params->accept(*this);
    }

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n");
    for (auto& STE : a.symbol_table.see_syms()) {
        print_STE(indent, STE);
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.block->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct MainNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Main: main\n");

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n");
    for (auto& STE : a.symbol_table.see_syms()) {
        print_STE(indent, STE);
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.block->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct BlockNode& a) {
    if (a.statements.empty()) return std::any{};
    print_indent(depth(a), "↪ Block\n");
    for (auto& s : a.statements) s->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct DeclarationNode& a) {
    print_indent(depth(a), "↪ Declaration: " + a.id + " : " + a.type + " : " + std::to_string(a.frame_offset) + '\n');
    return std::any{};
}
std::any Printer::visit(struct VarInitNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Variable Initialization\n");

    // print_indent(indent + (INDENT >> 1), "> Declaration\n");
    a.dcl->accept(*this);

    if (!a.val) return std::any{};
    // print_indent(indent + (INDENT >> 1), "> Initial Value\n");
    a.val->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct IfNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ If Tree\n");

    auto it = a.clauses.begin();
    print_indent(indent + (INDENT >> 1), "> IF\n");
    // print_indent(indent + INDENT, "> Condition\n");
    it->cond->accept(*this);
    // print_indent(indent + INDENT, "> Statements\n");
    it->block->accept(*this);

    it++;
    for (; it != a.clauses.end(); it++) {
         print_indent(indent + (INDENT >> 1), "> ");
        if (it->cond) {
            os << "ELIF\n";
            // print_indent(indent + INDENT, "> Condition\n");
            it->cond->accept(*this);
        }
        else {
            os << "ELSE\n";
        }
        // print_indent(indent + INDENT, "> Statements\n");
        it->block->accept(*this);
    }
    return std::any{};
}
std::any Printer::visit(struct DeleteNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Delete\n");

    // print_indent(indent + (INDENT >> 1), "> Pointer\n");
    a.ptr->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct PrintNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Print\n");

    // print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct ReturnNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Return\n");

    if (!a.expr) return std::any{};
    // print_indent(indent + (INDENT >> 1), "> Expr\n");
    a.expr->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct WhileNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ While\n");

    // print_indent(indent + (INDENT >> 1), "> Condition\n");
    a.condition->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.statements->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct AssignmentNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Assignment\n");

    print_indent(indent + (INDENT >> 1), "> LValue\n");
    a.LHS->accept(*this);

    print_indent(indent + (INDENT >> 1), "> New Value\n");
    a.RHS->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct ForNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ For\n");

    // print_indent(indent + (INDENT >> 1), "> For Prologue\n");
    a.prologue->accept(*this);

    print_indent(indent + (INDENT >> 1), "> Condition\n");
    a.cond->accept(*this);

    print_indent(indent + (INDENT >> 1), "> For Epilogue\n");
    a.epilogue->accept(*this);

    print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.block->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct BreakNode& a) {
    print_indent(depth(a), "↪ Break\n");
    return std::any{};
}
std::any Printer::visit(struct NumNode& a) {
    print_indent(depth(a), "↪ Integer : " + std::to_string(a.val) + '\n');
    return std::any{};
}
std::any Printer::visit(struct CharNode& a) {
    print_indent(depth(a), "↪ Character : ");
    os << a.val << '\n';
    return std::any{};
}
std::any Printer::visit(struct TrueNode& a) {
    print_indent(depth(a), "↪ Boolean : TRUE\n");
    return std::any{};
}
std::any Printer::visit(struct FalseNode& a) {
    print_indent(depth(a), "↪ Boolean : FALSE\n");
    return std::any{};
}
std::any Printer::visit(struct IDNode& a) {
    print_indent(depth(a), "↪ ID : " + a.name);
    if (!a.type.empty()) os << " : " << a.type;
    os << " : " << a.offset << '\n';
    return std::any{};
}
std::any Printer::visit(struct NilNode& a) {
    print_indent(depth(a), "↪ Pointer : NULL\n");
    return std::any{};
}
std::any Printer::visit(struct BinaryExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Binary Expression: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Left Side\n");
    a.LHS->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Right Side\n");
    a.RHS->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct MemberAccessExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Member Access: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this);

     print_indent(indent + (INDENT >> 1), "> Field : ");
     os << a.id + '\n';
    return std::any{};
}
std::any Printer::visit(struct UnaryExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Unary Expression: " << a.op;
    if (!a.type.empty()) oss << " : " << a.type;
    oss << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct AllocNode& a) {
    print_indent(depth(a), "↪ Allocation: " + a.ptr_type + " [ " + std::to_string(a.size) + " ]\n");
    return std::any{};
}
std::any Printer::visit(struct FunctionCallNode& a) {
    ASTNode* prog = nullptr;
    get_parent_nodes(&a, &prog);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);

    const size_t indent = depth(a);
    print_indent(indent, "↪ Function Call: " + a.id + " -> " + PROG->procedures.at(a.id)->return_type + '\n');

    if (!a.args) return std::any{};
//    print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this);
    return std::any{};
}
std::any Printer::visit(struct ReadCallNode& a) {
    std::ostringstream oss;
    oss << "↪ Function Call: read -> " << TYPE_CHAR << '\n';
    print_indent(depth(a), oss.str());
    return std::any{};
}
