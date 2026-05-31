#include "typecheck.h"
#include <sstream>
#include "visitor_helper.h"

// Implementation

void TypeChecker::visit(struct ArgsNode& a, std::ostream& os) {
    for (auto& arg : a.args) {
        arg->accept(*this, os);
    }

    auto* func_call = dynamic_cast<FunctionCallNode*>(a.parent);
    const std::string& id = func_call->id;

    ASTNode* parent = nullptr;
    get_parent_nodes(&a, &parent);
    auto* prog = dynamic_cast<ProgramNode*>(parent);

    if (!prog->procedures.contains(id)) throw std::runtime_error{"ERROR: Trying to call undefined procedure: " + id};
    const std::vector<std::unique_ptr<DeclarationNode>>& params = prog->procedures.at(id)->params->declarations;

    if (a.args.size() != params.size()) throw std::runtime_error{"ERROR: Expected " + std::to_string(params.size()) + " arguments for " + id + ", got " + std::to_string(a.args.size())};
    for (size_t i = 0; i < a.args.size(); i++) {
        if (a.args.at(i)->type != params.at(i)->type)
            throw std::runtime_error{"ERROR: Mismatching type for procedure call argument"};
    }
}
void TypeChecker::visit(struct DeclarationsNode& a, std::ostream& os) {}
void TypeChecker::visit(struct ForPrologueNode& a, std::ostream& os) {
    if (a.asst) a.asst->accept(*this, os);
    else if (a.init) a.init->accept(*this, os);
    else throw std::runtime_error{"ERROR: Invalid For Prologue"}; // shouldn't ever throw...
}
void TypeChecker::visit(struct ProgramNode& a, std::ostream& os) {
    for (auto& [id, sd] : a.struct_defs) {
        sd->accept(*this, os);
    }

    for (auto& init : a.global_vars) {
        init->accept(*this, os);
    }

    for (auto& [id, proc] : a.procedures) {
        proc->accept(*this, os);
    }

    a.main->accept(*this, os);
}
void TypeChecker::visit(struct StructDefNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    get_parent_nodes(&a, &prog);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    auto& sizes = PROG->type_sizes;

    int size = 0;
    for (auto& [id, dcl] : a.fields) {
        if (!sizes.contains(dcl->type) && !is_pointer(dcl->type))
            throw std::runtime_error{"ERROR: Undefined type used in definition of struct: " + a.id};
        const int dcl_size = is_pointer(dcl->type) ? sizes.at("*") : sizes.at(dcl->type);
        if (dcl_size >= SIZE_INT) size = (size + (SIZE_INT-1)) & ~(SIZE_INT-1);
        dcl->frame_offset = -size;
        size += dcl_size;
    }
    size = (size + 3) & ~3;
    sizes.insert({a.id, size});
}
void TypeChecker::visit(struct ProcedureNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    get_parent_nodes(&a, &prog);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);

    int size = 0;
    for (auto& dcl : a.params->declarations) {
        if (!is_pointer(dcl->type) && !PROG->type_sizes.contains(dcl->type))
            throw std::runtime_error{"ERROR: Trying to declare variable of undefined type: " + dcl->type};
        const int type_size = is_pointer(dcl->type) ? PROG->type_sizes.at("*") : PROG->type_sizes.at(dcl->type);
        a.symbol_table.declare_variable(dcl->id, dcl->type, type_size);
        dcl->frame_offset = type_size;
        size += type_size;
    }
    for (auto& dcl : a.params->declarations) {
        dcl->frame_offset += size;
    }

    a.block->accept(*this, os);
}
void TypeChecker::visit(struct MainNode& a, std::ostream& os) {
    a.block->accept(*this, os);
}
void TypeChecker::visit(struct BlockNode& a, std::ostream& os) {
    for (auto& statement : a.statements) {
        statement->accept(*this, os);
        if (statement->node_type == Parser::RETURN) return;
    }
}
void TypeChecker::visit(struct DeclarationNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, &prog, &proc);

    if (!proc) return;

    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    if (!is_pointer(a.type) && !PROG->type_sizes.contains(a.type))
        throw std::runtime_error{"ERROR: Trying to declare variable of undefined type: " + a.type};

    PROC->symbol_table.declare_variable(a.id, a.type,
        is_pointer(a.type) ? PROG->type_sizes.at("*") : PROG->type_sizes.at(a.type)
    );

    SymbolTableEntry* ste = PROC->symbol_table.lookup_variable(a.id);
    a.frame_offset = ste->frame_offset;
}
void TypeChecker::visit(struct VarInitNode& a, std::ostream& os) {
    a.dcl->accept(*this, os);
    if (!a.val) return;
    a.val->accept(*this, os);
    if (a.dcl->type != a.val->type) throw std::runtime_error{"ERROR: Trying to initialize mismatching values"};
}
void TypeChecker::visit(struct IfNode& a, std::ostream& os) {
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, nullptr, &proc);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    for (auto& clause : a.clauses) {
        if (clause.cond) clause.cond->accept(*this, os);
        PROC->symbol_table.enter_scope();
        clause.block->accept(*this, os);
        PROC->symbol_table.exit_scope();
    }
}
void TypeChecker::visit(struct DeleteNode& a, std::ostream& os) {
    a.ptr->accept(*this, os);
    if (!is_pointer(a.ptr->type)) throw std::runtime_error{"ERROR: Trying to delete on NON-PTR type"};
}
void TypeChecker::visit(struct PrintNode& a, std::ostream& os) {
    for (auto& arg : a.args->args) {
        arg->accept(*this, os);
        if (is_struct(arg->type)) throw std::runtime_error{"ERROR: Trying to print STRUCT type"};
    }
}
void TypeChecker::visit(struct ReturnNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, &prog, &proc);
    if (!proc)  throw std::runtime_error{"ERROR: Returning from outside a procedure"};

    if (proc->node_type == Parser::procedure) {
        auto* PROG = dynamic_cast<ProgramNode*>(prog);
        auto* PROC = dynamic_cast<ProcedureNode*>(proc);
        if (!a.expr && PROG->procedures.at(PROC->id)->return_type != "void")
            throw std::runtime_error{"ERROR: Returning nothing for NON-VOID procedure: " + PROC->id};
        if (!a.expr) return;
        if (PROG->procedures.at(PROC->id)->return_type == "void")
            throw std::runtime_error{"ERROR: Returning something for VOID function: " + PROC->id};
        a.expr->accept(*this, os);
        if (a.expr->type != PROG->procedures.at(PROC->id)->return_type)
            throw std::runtime_error{"ERROR: Returning mismatching type for procedure: " + PROC->id};
    }
    else { // Main Node
        if (!a.expr)
            throw std::runtime_error{"ERROR: Returning nothing for main procedure"};
        a.expr->accept(*this, os);
        if (a.expr->type != TYPE_INT)
            throw std::runtime_error{"ERROR: Returning NON-INT type for INT procedure: main"};
    }
}
void TypeChecker::visit(struct WhileNode& a, std::ostream& os) {
    a.condition->accept(*this, os);

    ASTNode* proc = nullptr;
    get_parent_nodes(&a, nullptr, &proc);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    PROC->symbol_table.enter_scope();
    a.statements->accept(*this, os);
    PROC->symbol_table.exit_scope();
}
void TypeChecker::visit(struct AssignmentNode& a, std::ostream& os) {
    a.LHS->accept(*this, os);
    if (!is_lvalue(a.LHS)) throw std::runtime_error{"ERROR: Trying to assign value to non-LVALUE"};
    a.RHS->accept(*this, os);
    if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to assign mismatching values"};
}
void TypeChecker::visit(struct ForNode& a, std::ostream& os) {
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, nullptr, &proc);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    PROC->symbol_table.enter_scope();
    a.prologue->accept(*this, os);
    a.cond->accept(*this, os);
    a.epilogue->accept(*this, os);
    a.block->accept(*this, os);
    PROC->symbol_table.exit_scope();
}
void TypeChecker::visit(struct BreakNode& a, std::ostream& os) {
    ASTNode* parent = a.parent;
    while (parent) {
        if (parent->node_type == Parser::FOR || parent->node_type == Parser::WHILE) {
            a.loop_target = parent;
            break;
        }
        parent = parent->parent;
    }
    if (!a.loop_target) throw std::runtime_error{"ERROR: Using BREAK statement outside of for/while-loop body"};
}
void TypeChecker::visit(struct NumNode& a, std::ostream& os) {}
void TypeChecker::visit(struct CharNode& a, std::ostream& os) {}
void TypeChecker::visit(struct TrueNode& a, std::ostream& os) {}
void TypeChecker::visit(struct FalseNode& a, std::ostream& os) {}
void TypeChecker::visit(struct IDNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, &prog, &proc);
    if (!proc) throw std::runtime_error{"ERROR: Unable to find procedure using variable: " + a.name};

    // Local Scope
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);
    SymbolTableEntry* ste = PROC->symbol_table.lookup_variable(a.name);
    if (ste) {
        a.type = ste->type;
        a.offset = ste->frame_offset;
        return;
    }

    // Global Scope
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    if (PROG->symbol_table.contains(a.name)) {
        a.type = PROG->symbol_table.at(a.name).type;
        return;
    }
    throw std::runtime_error{"ERROR: Unidentified variable: " + a.name + " in procedure: " + PROC->id};
}
void TypeChecker::visit(struct NilNode& a, std::ostream& os) {
    a.type = "*";
}
void TypeChecker::visit(struct BinaryExprNode& a, std::ostream& os) {
    a.LHS->accept(*this, os);
    a.RHS->accept(*this, os);
    switch (a.op) {
        case Parser::OR:
        case Parser::AND:
            if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to compare values of different types: " + a.LHS->type + " vs " + a.RHS->type};
            if (a.LHS->type != TYPE_BOOL) throw std::runtime_error{"ERROR: Trying to create Boolean statement from NON-BOOLEAN values"};
            a.type = TYPE_BOOL;
            break;
        case Parser::GEQ:
        case Parser::GT:
        case Parser::LEQ:
        case Parser::LT:
        case Parser::EQUALS:
        case Parser::NEQ:
            if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to compare values of different types: " + a.LHS->type + " vs " + a.RHS->type};
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
void TypeChecker::visit(struct MemberAccessExprNode& a, std::ostream& os) {
    a.arg->accept(*this, os);

    ASTNode* prog = nullptr;
    get_parent_nodes(&a, &prog);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);

    if (!is_struct(a.arg->type))
        throw std::runtime_error{"ERROR: De-referencing variable of unknown STRUCT type"};

    switch (a.op) {
        case Parser::ARROW: {
            if(!is_pointer(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Pointer Reference operator -> on NON-PTR type"};
            if(!is_struct_pointer(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Pointer Reference operator -> on NON-STRUCT@ type"};
            const std::string struct_type = a.arg->type.substr(0, a.arg->type.size() - 1);
            if (!PROG->struct_defs.contains(struct_type))
                throw std::runtime_error{"ERROR: De-referencing variable pointer of unknown STRUCT type"};
            const auto& sd = PROG->struct_defs.at(struct_type);
            if (!sd->fields.contains(a.id))
                throw std::runtime_error{"ERROR: Attempting to read undefined field of struct: " + struct_type};
            a.type = sd->fields.at(a.id)->type;
            break;
        }
        case Parser::DOT: {
            if(is_pointer(a.arg->type))
                throw std::runtime_error{"ERROR: Attempting to use Struct Reference operator . on PTR type"};
            if (!PROG->struct_defs.contains(a.arg->type))
                throw std::runtime_error{"ERROR: De-referencing variable struct of unknown STRUCT type"};
            const auto& sd = PROG->struct_defs.at(a.arg->type);
            if (!sd->fields.contains(a.id))
                throw std::runtime_error{"ERROR: Attempting to read undefined field of struct: " + a.arg->type};
            a.type = sd->fields.at(a.id)->type;
            break;
        }
        default:
            throw std::runtime_error{"ERROR: Invalid Member Access Operator Found"};
    }
}
void TypeChecker::visit(struct UnaryExprNode& a, std::ostream& os) {
    a.arg->accept(*this, os);
    switch (a.op) {
        case Parser::NOT:
            if (a.arg->type != TYPE_BOOL) throw std::runtime_error{"ERROR: Attempting to use BOOL operator ! on NON-BOOL type"};
            a.type = TYPE_BOOL;
            break;
        case Parser::expr3:
        case Parser::expr4:
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
void TypeChecker::visit(struct AllocNode& a, std::ostream& os) {
    a.type = a.ptr_type;
}
void TypeChecker::visit(struct FunctionCallNode& a, std::ostream& os) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    get_parent_nodes(&a, &prog, &proc);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    std::ostringstream oss;

    // Check local scope
    SymbolTableEntry* ste = PROC->symbol_table.lookup_variable(a.id);
    if (ste)
        throw std::runtime_error{"ERROR: Trying to call procedure with same name as local variable: " + a.id};

    // Check global scope
    if (PROG->symbol_table.contains(a.id))
        throw std::runtime_error{"ERROR: Trying to call procedure with same name as global variable: " + a.id};

    if (!PROG->procedures.contains(a.id)) throw std::runtime_error{"ERROR: Trying to call undefined procedure: " + a.id};
    a.type = PROG->procedures.at(a.id)->return_type;

    if (a.args) a.args->accept(*this, os);
}
void TypeChecker::visit(struct ReadCallNode& a, std::ostream& os) {
    a.type = TYPE_CHAR;
}
