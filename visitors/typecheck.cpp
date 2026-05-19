#include "typecheck.h"
#include <sstream>

bool is_pointer(const std::string& type) { return type.ends_with('*'); }

bool is_lvalue(const std::unique_ptr<ExprNode>& node) {
    // just check for presence of ID node
    return false;
}

bool is_struct(const std::string& type) { return type.starts_with("struct"); }

bool is_struct_pointer(const std::string& type) {
    if (!type.starts_with("struct")) return false;
    const char secondLast = *std::next(type.rbegin());
    if (secondLast == '*') return false;
    return true;
}

void TypeChecker::visit(struct ArgsNode& a) {
    for (auto& arg : a.args) {
        arg->accept(*this);
    }
    // TODO, compare with procedure signature
}
void TypeChecker::visit(struct DeclarationsNode& a) {}
void TypeChecker::visit(struct ForPrologueNode& a) {
    if (a.asst) a.asst->accept(*this);
    else if (a.init) a.init->accept(*this);
    else throw std::runtime_error{"ERROR: Invalid For Prologue"};
}
void TypeChecker::visit(struct ProgramNode& a) {
    a.main->accept(*this);
    for (auto& [id, proc] : a.procedures) {
        proc->accept(*this);
    }
    for (auto& init : a.global_vars) {
        init->accept(*this);
    }
}
void TypeChecker::visit(struct StructDefNode& a) {}
void TypeChecker::visit(struct ProcedureNode& a) {
    a.block->accept(*this);
}
void TypeChecker::visit(struct MainNode& a) {
    a.block->accept(*this);
}
void TypeChecker::visit(struct BlockNode& a) {
    for (auto& statement : a.statements) {
        statement->accept(*this);
    }
}
void TypeChecker::visit(struct DeclarationNode& a) {}
void TypeChecker::visit(struct VarInitNode& a) {
    if (a.val) {
        a.val->accept(*this);
        if (a.dcl->type != a.val->type) throw std::runtime_error{"ERROR: Trying to initialize mismatching values"};
    }
}
void TypeChecker::visit(struct IfNode& a) {
    for (auto& clause : a.clauses) {
        clause.cond->accept(*this);
        clause.block->accept(*this);
    }
}
void TypeChecker::visit(struct DeleteNode& a) {
    a.ptr->accept(*this);
    if (!is_pointer(a.ptr->type)) throw std::runtime_error{"ERROR: Trying to delete on NON-PTR type"};
}
void TypeChecker::visit(struct PrintNode& a) {
    // TODO
}
void TypeChecker::visit(struct ReturnNode& a) {
    if (!a.expr) return;
    a.expr->accept(*this);
    ASTNode* parent = a.parent;
    while (parent && parent->node_type != Parser::ParserSymbol::start)
        parent = parent->parent;
    auto* prog = dynamic_cast<ProgramNode*>(parent);
//    if (a.expr->type != prog->)
}
void TypeChecker::visit(struct WhileNode& a) {
    a.condition->accept(*this);
    a.statements->accept(*this);
}
void TypeChecker::visit(struct AssignmentNode& a) {
    a.LHS->accept(*this);
    a.RHS->accept(*this);
    if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to assign mismatching values"};
}
void TypeChecker::visit(struct ForNode& a) {
    a.prologue->accept(*this);
    a.cond->accept(*this);
    a.prologue->accept(*this);
    a.block->accept(*this);
}
void TypeChecker::visit(struct BreakNode& a) {}
void TypeChecker::visit(struct NumNode& a) {
    a.type = TYPE_INT;
}
void TypeChecker::visit(struct CharNode& a) {
    a.type = TYPE_CHAR;
}
void TypeChecker::visit(struct TrueNode& a) {
    a.type = TYPE_BOOL;
}
void TypeChecker::visit(struct FalseNode& a) {
    a.type = TYPE_BOOL;
}
void TypeChecker::visit(struct IDNode& a) {
    ASTNode* prog = a.parent;
    ASTNode* proc = nullptr;
    ASTNode* scope = nullptr;
    while (prog->node_type != Parser::ParserSymbol::start) {
        if (!scope && (prog->node_type == Parser::ParserSymbol::WHILE || prog->node_type == Parser::ParserSymbol::FOR ||
                       prog->node_type == Parser::ParserSymbol::procedure))
            scope = prog;
        if (!proc && prog->node_type == Parser::ParserSymbol::procedure)
            proc = prog;
        prog = prog->parent;
    }
    std::ostringstream oss;
    oss << a.name << scope;
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);
    if (PROC->symbol_table.contains(oss.str()) && PROC->symbol_table.at(oss.str()).scope == scope) {
        a.type = PROC->symbol_table.at(oss.str()).type;
    }
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    if (PROG->symbol_table.contains(oss.str())) {
        a.type = PROG->symbol_table.at(oss.str()).type;
    }
    throw std::runtime_error{"ERROR: Unidentified variable: " + a.name};
}
void TypeChecker::visit(struct NilNode& a) {
    a.type = "*";
}
void TypeChecker::visit(struct BinaryExprNode& a) {
    a.LHS->accept(*this);
    a.RHS->accept(*this);
    switch (a.op) {
        case Parser::OR:
        case Parser::AND:
            if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to compare values of different types"};
            if (a.LHS->type != TYPE_BOOL) throw std::runtime_error{"ERROR: Trying to create Boolean statement from NON-BOOLEAN values"};
            a.type = TYPE_BOOL;
            break;
        case Parser::GEQ:
        case Parser::GT:
        case Parser::LEQ:
        case Parser::LT:
        case Parser::EQUALS:
        case Parser::NEQ:
            if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to compare values of different types"};
            if (a.LHS->type != TYPE_INT && a.LHS->type != TYPE_CHAR && a.LHS->type != TYPE_BOOL) throw std::runtime_error{"ERROR: Trying to create Boolean condition from non-elementary types"};
            a.type = TYPE_BOOL;
            break;
        case Parser::PLUS:
        case Parser::SUB:
            if (a.LHS->type != TYPE_INT && a.RHS->type != TYPE_INT) throw std::runtime_error{"ERROR: Trying to perform Addition/Subtraction on NON-INT types"};
            // from here, AT LEAST ONE is an INT
            if (a.LHS->type == TYPE_INT && a.RHS->type == TYPE_INT) a.type = TYPE_INT;
            // from here, ONE is an INT, OTHER is NOT, it better be a pointer!
            else if (is_pointer(a.LHS->type) || is_pointer(a.RHS->type)) a.type = is_pointer(a.RHS->type) ? a.RHS->type : a.LHS->type;
            else throw std::runtime_error{"ERROR: Trying to perform Addition/Subtraction on a NON-INT and NON-PTR type"};
            break;
        case Parser::MULT:
        case Parser::DIV:
        case Parser::MOD:
        case Parser::LSHIFT:
        case Parser::RSHIFT:
        case Parser::EXP:
        case Parser::BITOR:
        case Parser::BITXOR:
        case Parser::BITAND:
        case Parser::BITNOT:
            if (a.LHS->type != TYPE_INT || a.RHS->type != TYPE_INT) throw std::runtime_error{"ERROR: Trying to perform *,/,%,<<,>>,^^,|,^,&,~ on NON-INT types"};
            a.type = TYPE_INT;
            break;
        default:
            throw std::runtime_error{"ERROR: Invalid Binary Expression Operator Found"};
    }
}
void TypeChecker::visit(struct MemberAccessExprNode& a) {
    a.arg->accept(*this);
    switch (a.op) {
        case Parser::ARROW:
            if (!is_lvalue(a.arg) || !is_pointer(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Pointer Reference operator -> on NON-PTR type"};
            if (!is_struct_pointer(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Pointer Reference operator -> on NON-STRUCT type"};
            // NEED SYMBOL TABLE TODO
            break;
        case Parser::DOT:
            if (!is_lvalue(a.arg) || !is_struct(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Struct Reference operator . on NON-STRUCT type"};
            // NEED SYMBOL TABLE TODO
            break;
        default:
            throw std::runtime_error{"ERROR: Invalid Member Access Operator Found"};
    }
}
void TypeChecker::visit(struct UnaryExprNode& a) {
    a.accept(*this);
    switch (a.op) {
        case Parser::NOT:
            if (a.arg->type != TYPE_BOOL) throw std::runtime_error{"ERROR: Attempting to use BOOL operator ! on NON-BOOL type"};
            a.type = TYPE_BOOL;
            break;
        case Parser::PLUS:
        case Parser::SUB:
        case Parser::BITNOT:
            if (a.arg->type != TYPE_INT) throw std::runtime_error{"ERROR: Invalid Unary Expression Operator used on NON-INT type"};
            a.type = a.arg->type;
            break;
        case Parser::INCR:
        case Parser::DECR:
            if (a.arg->type != TYPE_INT && !is_pointer(a.arg->type)) throw std::runtime_error{"ERROR: Invalid Unary Expression Operator used on NON-INT && NON-PTR type"};
            break;
        case Parser::AT: {
            if (!is_pointer(a.arg->type)) throw std::runtime_error{"ERROR: Attempting to de-reference NON-POINTER type"};
            std::string type = a.arg->type;
            type.pop_back();
            a.type = std::move(type);
            break;
        }
        case Parser::ADDR:
            if (!is_lvalue(a.arg)) throw std::runtime_error{"ERROR: Attempting to reference NON-LVALUE"};
            a.type = a.arg->type + '*';
            break;
        default:
            throw std::runtime_error{"ERROR: Invalid Unary Expression Operator Found"};
    }
}
void TypeChecker::visit(struct AllocNode& a) {
    a.type = a.ptr_type;
}
void TypeChecker::visit(struct FunctionCallNode& a) {
    // TODO
}
void TypeChecker::visit(struct ReadCallNode& a) {
    // TODO
}
