#include "typecheck.h"
#include <sstream>

// Helpers

bool is_pointer(const std::string& type) { return type.ends_with('*'); }

bool is_lvalue(const std::unique_ptr<ExprNode>& node) {
    switch (node->node_type) {
        case Parser::ID:
        case Parser::DOT:
        case Parser::ARROW:
            return true;
        case Parser::NUM:
        case Parser::CHARLIT:
        case Parser::TRUE:
        case Parser::FALSE:
        case Parser::NIL:
        case Parser::paramlist:
        case Parser::NEW:
            return false;
        case Parser::OR:
        case Parser::AND:
        case Parser::BITOR:
        case Parser::BITXOR:
        case Parser::BITAND:
        case Parser::EQUALS:
        case Parser::NEQ:
        case Parser::LT:
        case Parser::LEQ:
        case Parser::GT:
        case Parser::GEQ:
        case Parser::LSHIFT:
        case Parser::RSHIFT:
        case Parser::PLUS:
        case Parser::SUB:
        case Parser::MULT:
        case Parser::DIV:
        case Parser::MOD:
        case Parser::EXP: {
            auto* bn = dynamic_cast<BinaryExprNode*>(node.get());
            return is_lvalue(bn->LHS) || is_lvalue(bn->RHS);
        }
        case Parser::AT:
        case Parser::ADDR:
        case Parser::NOT:
        case Parser::BITNOT:
        case Parser::INCR:
        case Parser::DECR:
        case Parser::expr3:
        case Parser::expr4: {
            auto* un = dynamic_cast<UnaryExprNode*>(node.get());
            return is_lvalue(un->arg);
        }
        default:
            return false;
    }
}

bool is_struct(const std::string& type) {
    return !type.starts_with(TYPE_INT) && !type.starts_with(TYPE_CHAR) && !type.starts_with(TYPE_BOOL);
}

bool is_struct_pointer(const std::string& type) {
    if (!is_struct(type)) return false;
    const char secondLast = *std::next(type.rbegin());
    if (secondLast == '*') return false;
    return true;
}

inline bool is_proc_node(Parser::ParserSymbol ps) {
    return ps == Parser::MAIN || ps == Parser::procedure;
}

inline bool is_scopable(Parser::ParserSymbol ps) {
    return is_proc_node(ps) || ps == Parser::FOR || ps == Parser::WHILE || ps == Parser::IF;
}

void get_parent_nodes(ASTNode* start, ASTNode** prog = nullptr, ASTNode** proc = nullptr, ASTNode** scope = nullptr) {
    ASTNode* curr = start;
    while (curr->parent) {
        if (proc && !*proc && is_proc_node(curr->node_type)) *proc = curr;
        if (scope && !*scope && is_scopable(curr->node_type)) *scope = curr;
        curr = curr->parent;
    }
    if (prog) *prog = curr;
}

// Implementation

void TypeChecker::visit(struct ArgsNode& a) {
    for (auto& arg : a.args) {
        arg->accept(*this);
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
void TypeChecker::visit(struct DeclarationsNode& a) {}
void TypeChecker::visit(struct ForPrologueNode& a) {
    if (a.asst) a.asst->accept(*this);
    else if (a.init) a.init->accept(*this);
    else throw std::runtime_error{"ERROR: Invalid For Prologue"}; // shouldn't ever throw...
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
        if (statement->node_type == Parser::RETURN) return;
    }
}
void TypeChecker::visit(struct DeclarationNode& a) {
    ASTNode* proc = nullptr;
    ASTNode* scope = nullptr;
    get_parent_nodes(&a, nullptr, &proc, &scope);
    if (!proc) return;
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    std::ostringstream oss;
    oss << a.id << scope;
    if (PROC->symbol_table.contains(oss.str())) throw std::runtime_error{"ERROR: Redeclaration of variable: " + a.id};

    SymbolTableEntry ste{.id = a.id, .type = a.type, .scope = scope};
    PROC->symbol_table.insert({oss.str(), ste});
}
void TypeChecker::visit(struct VarInitNode& a) {
    a.dcl->accept(*this);
    if (!a.val) return;
    a.val->accept(*this);
    if (a.dcl->type != a.val->type) throw std::runtime_error{"ERROR: Trying to initialize mismatching values"};
}
void TypeChecker::visit(struct IfNode& a) {
    for (auto& clause : a.clauses) {
        if (clause.cond) clause.cond->accept(*this);
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
        a.expr->accept(*this);
        if (a.expr->type != PROG->procedures.at(PROC->id)->return_type)
            throw std::runtime_error{"ERROR: Returning mismatching type for procedure: " + PROC->id};
    }
    else { // Main Node
        if (!a.expr)
            throw std::runtime_error{"ERROR: Returning nothing for main procedure"};
        a.expr->accept(*this);
        if (a.expr->type != TYPE_INT)
            throw std::runtime_error{"ERROR: Returning NON-INT type for INT procedure: main"};
    }
}
void TypeChecker::visit(struct WhileNode& a) {
    a.condition->accept(*this);
    a.statements->accept(*this);
}
void TypeChecker::visit(struct AssignmentNode& a) {
    a.LHS->accept(*this);
    if (!is_lvalue(a.LHS)) throw std::runtime_error{"ERROR: Trying to assign value to non-LVALUE"};
    a.RHS->accept(*this);
    if (a.LHS->type != a.RHS->type) throw std::runtime_error{"ERROR: Trying to assign mismatching values"};
}
void TypeChecker::visit(struct ForNode& a) {
    a.prologue->accept(*this);
    a.cond->accept(*this);
    a.epilogue->accept(*this);
    a.block->accept(*this);
}
void TypeChecker::visit(struct BreakNode& a) {
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
void TypeChecker::visit(struct NumNode& a) {}
void TypeChecker::visit(struct CharNode& a) {}
void TypeChecker::visit(struct TrueNode& a) {}
void TypeChecker::visit(struct FalseNode& a) {}
void TypeChecker::visit(struct IDNode& a) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    ASTNode* scope = nullptr;
    get_parent_nodes(&a, &prog, &proc, &scope);
    if (!proc) throw std::runtime_error{"ERROR: Unable to find procedure using variable: " + a.name};

    std::ostringstream oss;
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    // Local Scope
    oss << a.name << scope;
    if (PROC->symbol_table.contains(oss.str()) && PROC->symbol_table.at(oss.str()).scope == scope) {
        a.type = PROC->symbol_table.at(oss.str()).type;
        return;
    }

    // Procedure Scope
    if (PROC != scope) {
        oss.str("");
        oss << a.name << PROC;
        if (PROC->symbol_table.contains(oss.str()) && PROC->symbol_table.at(oss.str()).scope == PROC) {
            a.type = PROC->symbol_table.at(oss.str()).type;
            return;
        }
    }

    // Global Scope
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    oss.str("");
    oss << a.name << PROG;
    if (PROG->symbol_table.contains(oss.str())) {
        a.type = PROG->symbol_table.at(oss.str()).type;
        return;
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
void TypeChecker::visit(struct UnaryExprNode& a) {
    a.arg->accept(*this);
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
void TypeChecker::visit(struct AllocNode& a) {
    a.type = a.ptr_type;
}
void TypeChecker::visit(struct FunctionCallNode& a) {
    ASTNode* prog = nullptr;
    ASTNode* proc = nullptr;
    ASTNode* scope = nullptr;
    get_parent_nodes(&a, &prog, &proc, &scope);
    auto* PROG = dynamic_cast<ProgramNode*>(prog);
    auto* PROC = dynamic_cast<ProcedureNode*>(proc);

    std::ostringstream oss;

    // Check local scope
    oss << a.id << scope;
    if (PROC->symbol_table.contains(oss.str()))
        throw std::runtime_error{"ERROR: Trying to call procedure with same name as local variable: " + a.id};
    oss.str("");

    // Check procedure scope
    if (PROC != scope) {
        oss << a.id << PROC;
        if (PROC->symbol_table.contains(oss.str()))
            throw std::runtime_error{"ERROR: Trying to call procedure with same name as procedure variable: " + a.id};
        oss.str("");
    }

    // Check global scope
    oss << a.id << PROG;
    if (PROG->symbol_table.contains(oss.str()))
        throw std::runtime_error{"ERROR: Trying to call procedure with same name as global variable: " + a.id};

    if (!PROG->procedures.contains(a.id)) throw std::runtime_error{"ERROR: Trying to call undefined procedure: " + a.id};
    a.type = PROG->procedures.at(a.id)->return_type;

    if (a.args) a.args->accept(*this);
}
void TypeChecker::visit(struct ReadCallNode& a) {
    a.type = TYPE_CHAR;
}
