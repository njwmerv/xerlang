#include "printer.h"
#include <iostream>
#include <sstream>

#define INDENT 4

size_t depth(const struct ASTNode& ast) {
    size_t depth = 0;
    const ASTNode* a = &ast;
    while ((a = a->parent)) depth++;
    return INDENT * depth;
}

void print_indent(size_t indent, const std::string& message) {
    for (size_t i = 1; i < indent; i++) std::cout << ' ';
    std::cout << message;
}

void Printer::visit(struct ArgsNode& a) {
    print_indent(depth(a), "↪ Args\n");
    for (auto& arg : a.args) arg->accept(*this);
}
void Printer::visit(struct DeclarationsNode& a) {
    if (a.declarations.empty()) return;
    print_indent(depth(a), "↪ Declarations\n");
    for (auto& dec : a.declarations) dec->accept(*this);
}
void Printer::visit(struct ForPrologueNode& a) {
    print_indent(depth(a), "↪ For Prologue\n");
    if (a.init) a.init->accept(*this);
    if (a.asst) a.asst->accept(*this);
}
void Printer::visit(struct ProgramNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Program\n");

    print_indent(indent + (INDENT >> 1), "> Struct Definitions\n");
    for (auto& sd : a.struct_defs) sd->accept(*this);

    print_indent(indent + (INDENT >> 1), "> Global Vars\n");
    for (auto& gv : a.global_vars) gv->accept(*this);

    print_indent(indent + (INDENT >> 1), "> Procedures\n");
    for (auto& proc : a.procedures) proc->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Main\n");
    a.main->accept(*this);
}
void Printer::visit(struct StructDefNode& a) {
    print_indent(depth(a), "↪ Struct Definition\n");
    a.fields->accept(*this);
}
void Printer::visit(struct ProcedureNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Procedure: " + a.id + " -> " + a.return_type + '\n');

    if (!a.params->declarations.empty()) {
        print_indent(indent + (INDENT >> 1), "> Parameters\n");
        a.params->accept(*this);
    }

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n");
    for (auto& [name, STE] : a.symbol_table) {
        print_indent(indent + INDENT, "> " + name + '\n');
        // TODO: modify this to show SymbolTableEntry
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.block->accept(*this);
}
void Printer::visit(struct MainNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Main: main\n");

    print_indent(indent + (INDENT >> 1), "> Symbol Table\n");
    for (auto& [name, STE] : a.symbol_table) {
        print_indent(indent + INDENT, "> " + name + '\n');
        // TODO: modify this to show SymbolTableEntry
    }

    print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.block->accept(*this);
}
void Printer::visit(struct BlockNode& a) {
    print_indent(depth(a), "↪ Block\n");
    for (auto& s : a.statements) s->accept(*this);
}
void Printer::visit(struct DeclarationNode& a) {
    print_indent(depth(a), "↪ Declaration: " + a.id + " : " + a.type + '\n');
}
void Printer::visit(struct VarInitNode& a) {
    const size_t indent = depth(a);

    print_indent(indent, "↪ Variable Initialization\n");

    // print_indent(indent + (INDENT >> 1), "> Declaration\n");
    a.dcl->accept(*this);

    if (!a.val) return;
    // print_indent(indent + (INDENT >> 1), "> Initial Value\n");
    a.val->accept(*this);
}
void Printer::visit(struct IfNode& a) {
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
            std::cout << "ELIF\n";
            // print_indent(indent + INDENT, "> Condition\n");
            it->cond->accept(*this);
        }
        else {
            std::cout << "ELSE\n";
        }
        // print_indent(indent + INDENT, "> Statements\n");
        it->block->accept(*this);
    }
}
void Printer::visit(struct DeleteNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Delete\n");

    // print_indent(indent + (INDENT >> 1), "> Pointer\n");
    a.ptr->accept(*this);
}
void Printer::visit(struct PrintNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Print\n");

    // print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this);
}
void Printer::visit(struct ReturnNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Return\n");

    if (!a.expr) return;
    // print_indent(indent + (INDENT >> 1), "> Expr\n");
    a.expr->accept(*this);
}
void Printer::visit(struct WhileNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ While\n");

    // print_indent(indent + (INDENT >> 1), "> Condition\n");
    a.condition->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Statements\n");
    a.statements->accept(*this);
}
void Printer::visit(struct AssignmentNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Assignment\n");

     print_indent(indent + (INDENT >> 1), "> LValue\n");
    a.LHS->accept(*this);

     print_indent(indent + (INDENT >> 1), "> New Value\n");
    a.RHS->accept(*this);
}
void Printer::visit(struct ForNode& a) {
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
}
void Printer::visit(struct BreakNode& a) {
    print_indent(depth(a), "↪ Break\n");
}
void Printer::visit(struct NumNode& a) {
    print_indent(depth(a), "↪ Integer : " + std::to_string(a.val) + '\n');
}
void Printer::visit(struct CharNode& a) {
    print_indent(depth(a), "↪ Character : ");
    std::cout << a.val << '\n';
}
void Printer::visit(struct TrueNode& a) {
    print_indent(depth(a), "↪ Boolean : TRUE\n");
}
void Printer::visit(struct FalseNode& a) {
    print_indent(depth(a), "↪ Boolean : FALSE\n");
}
void Printer::visit(struct IDNode& a) {
    print_indent(depth(a), "↪ ID : " + a.name);
    if (!a.type.empty()) std::cout << " : " << a.type;
    std::cout << '\n';
}
void Printer::visit(struct NilNode& a) {
    print_indent(depth(a), "↪ Pointer : NULL\n");
}
void Printer::visit(struct BinaryExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Binary Expression: " << a.op << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Left Side\n");
    a.LHS->accept(*this);

    // print_indent(indent + (INDENT >> 1), "> Right Side\n");
    a.RHS->accept(*this);
}
void Printer::visit(struct MemberAccessExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Member Access: " << a.op << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this);

     print_indent(indent + (INDENT >> 1), "> Field : ");
     std::cout << a.id + '\n';
}
void Printer::visit(struct UnaryExprNode& a) {
    const size_t indent = depth(a);
    std::ostringstream oss;
    oss << "↪ Unary Expression: " << a.op << '\n';
    print_indent(indent, oss.str());

    // print_indent(indent + (INDENT >> 1), "> Argument\n");
    a.arg->accept(*this);
}
void Printer::visit(struct AllocNode& a) {
    print_indent(depth(a), "↪ Allocation: " + a.ptr_type + " [ " + std::to_string(a.size) + " ]\n");
}
void Printer::visit(struct FunctionCallNode& a) {
    const size_t indent = depth(a);
    print_indent(indent, "↪ Function Call: " + a.id + '\n');

    if (!a.args) return;
    print_indent(indent + (INDENT >> 1), "> Arguments\n");
    a.args->accept(*this);
}
void Printer::visit(struct ReadCallNode& a) {
    print_indent(depth(a), "↪ Function Call: read\n");
}
