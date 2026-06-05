#include "codegen.h"

#include <iostream>

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>

#include "visitor_helper.h"

// Helpers

llvm::Value* CodeGen::get_LValue(ASTNode* node) {
    // Case: x++
    if (auto* id = dynamic_cast<IDNode*>(node)) {
        if (named_values.contains(id->name)) {
            return named_values.at(id->name);
        }
        if (llvm::GlobalVariable* global_var = Module->getNamedGlobal(id->name)) {
            return global_var;
        }
        return nullptr;
    }

    // Case: *(ptr + 1)++
    if (auto* unary = dynamic_cast<UnaryExprNode*>(node)) {
        if (unary->op == Parser::ParserSymbol::AT) {
            return std::any_cast<llvm::Value*>(unary->arg->accept(*this));
        }
    }

    // Case: x.a++ / x->a++
    if (auto* member = dynamic_cast<MemberAccessExprNode*>(node)) {
        llvm::Value* struct_ptr = nullptr;
        std::string struct_name;

        if (member->op == Parser::ParserSymbol::DOT) {
            // For DOT, the LHS MUST be an L-Value (e.g., a local struct variable)
            struct_ptr = get_LValue(member->arg.get());
            if (!struct_ptr) return nullptr;

            struct_name = member->arg->type;
        }
        else if (member->op == Parser::ParserSymbol::ARROW) {
            // For ARROW, the LHS is an R-Value pointer expression.
            // We just evaluate it to get the pointer value itself!
            struct_ptr = std::any_cast<llvm::Value*>(member->arg->accept(*this));
            if (!struct_ptr) return nullptr;

            struct_name = member->arg->type;
            if (struct_name.ends_with('*')) struct_name.pop_back(); // Strip the pointer '*'
        }

        llvm::Type* opaque_type = get_LLVM_type(struct_name);
        auto* struct_type = llvm::cast<llvm::StructType>(opaque_type);

        if (!struct_field_indices.contains(struct_name) || !struct_field_indices.at(struct_name).contains(member->id)) {
            throw std::runtime_error{"ERROR: Unknown field: " + member->id + " in struct " + struct_name};
        }
        size_t field_index = struct_field_indices.at(struct_name).at(member->id);

        // Generate the offset from the base struct pointer
        return Builder.CreateStructGEP(struct_type, struct_ptr, field_index, "member_ptr_tmp");
    }

    throw std::runtime_error{"Error: Expression is not assignable (not an L-Value)."};
}

llvm::Type* CodeGen::get_LLVM_type(const std::string& type) {
    if (type == TYPE_INT) return Builder.getInt32Ty();
    if (type == TYPE_CHAR) return Builder.getInt8Ty();
    if (type == TYPE_BOOL) return Builder.getInt1Ty();
    if (type == TYPE_VOID) return Builder.getVoidTy();
    if (type.ends_with('*')) return Builder.getInt8PtrTy();

    if (struct_types.contains(type))
        return struct_types.at(type);

    throw std::runtime_error{"ERROR: Unknown type: " + type};
}

void CodeGen::print_ir(llvm::raw_fd_ostream& dest) {
    Module->print(dest, nullptr);
}

void CodeGen::optimize() {
    // 1. Initialize the Analysis Managers
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // 2. Create the Pass Builder
    llvm::PassBuilder PB;

    // 3. Register all the basic analyses with the managers
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // 4. Build the Optimization Pipeline (O2 is a great default)
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);

    // 5. Run the passes on your Module
    MPM.run(*Module, MAM);
}

// Implementation

std::any CodeGen::visit(struct ArgsNode& a) { return std::any{}; }
std::any CodeGen::visit(struct DeclarationsNode& a) {
    for (auto& decl : a.declarations) decl->accept(*this);
    return std::any{};
}
std::any CodeGen::visit(struct ForPrologueNode& a) {
    if (a.init) a.init->accept(*this);
    else if (a.asst) a.asst->accept(*this);
    return std::any{};
}
std::any CodeGen::visit(struct ProgramNode& a) {
    for (auto& [id, struct_def] : a.struct_defs) {
        struct_def->accept(*this);
    }
    for (auto& global : a.global_vars) {
        global->accept(*this);
    }
    for (auto& [id, proc] : a.procedures) {
        proc->accept(*this);
    }
    if (a.main) a.main->accept(*this);

    return std::any{};
}
std::any CodeGen::visit(struct StructDefNode& a) {
    llvm::StructType* structType = llvm::StructType::create(*Context, a.id);
    struct_types.insert({a.id, structType});
    struct_field_indices.insert({a.id, {}});

    size_t index = 0;
    std::vector<llvm::Type*> field_types;
    for (const auto& [field_name, dclNode] : a.fields) {
        std::string field_type_name = dclNode->type;
        llvm::Type* llvm_field_type = get_LLVM_type(field_type_name);

        if (!llvm_field_type)
            throw std::runtime_error{"CodeGen Error: Unknown type \'" + field_type_name + "\' in struct \'" + a.id + "\'\n"};

        struct_field_indices.at(a.id).insert({field_name, index});
        index++;

        field_types.push_back(llvm_field_type);
    }

    structType->setBody(field_types, false);

    return std::any{};
}
std::any CodeGen::visit(struct ProcedureNode& a) {
    // 1. Gather the return type and argument types
    llvm::Type* ret_type = get_LLVM_type(a.return_type);
    std::vector<llvm::Type*> arg_types;
    for (const auto& param : a.params->declarations) {
        arg_types.push_back(get_LLVM_type(param->type));
    }

    // 2. Create the function
    llvm::FunctionType* func_type = llvm::FunctionType::get(ret_type, arg_types, false);
    llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, a.id, Module.get());

    // 3. Set up the entry block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*Context, "entry", func);
    Builder.SetInsertPoint(entry);

    // 4. Create a new variable scope for this function
    auto old_named_values = named_values;

    // 5. Allocate arguments on the stack and save them to 'named_values'
    size_t idx = 0;
    for (auto& arg : func->args()) {
        std::string arg_name = a.params->declarations.at(idx)->id;
        arg.setName(arg_name);

        llvm::AllocaInst* alloca = Builder.CreateAlloca(arg_types[idx], nullptr, arg_name);
        Builder.CreateStore(&arg, alloca);

        named_values[arg_name] = alloca;
        idx++;
    }

    a.block->accept(*this);

    // Missing returns (e.g. -> void)
    if (!Builder.GetInsertBlock()->getTerminator()) {
        if (ret_type->isVoidTy()) {
            Builder.CreateRetVoid();
        }
        else if (ret_type->isIntegerTy()) {
            Builder.CreateRet(llvm::ConstantInt::get(ret_type, 0));
        }
        else if (ret_type->isPointerTy()) {
            auto* ptr_type = llvm::cast<llvm::PointerType>(ret_type);
            Builder.CreateRet(llvm::ConstantPointerNull::get(ptr_type));
        }
        else {
            throw std::runtime_error{"ERROR: Missing return statement for non-elementary type procedure: " + a.id};
        }
    }

    // 8. Restore the previous scope
    named_values = old_named_values;

    return std::any{};
}
std::any CodeGen::visit(struct MainNode& a) {
    llvm::FunctionType* func_type = llvm::FunctionType::get(Builder.getInt32Ty(), false);
    llvm::Function* main_func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", Module.get());

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*Context, "entry", main_func);
    Builder.SetInsertPoint(entry);

    a.block->accept(*this);

    // Implicit `return 0;` if not specified
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateRet(Builder.getInt32(0));

    return std::any{};
}
std::any CodeGen::visit(struct BlockNode& a) {
    auto old_named_values = named_values;
    for (auto& statement : a.statements) {
        statement->accept(*this);
    }

    named_values = old_named_values;
    return std::any{};
}
std::any CodeGen::visit(struct DeclarationNode& a) {
    llvm::Type* var_type = get_LLVM_type(a.type);

    // Global Scope
    if (!Builder.GetInsertBlock()) {
        Module->getOrInsertGlobal(a.id, var_type);
        llvm::GlobalVariable* g_var = Module->getNamedGlobal(a.id);

        g_var->setLinkage(llvm::GlobalValue::CommonLinkage);

        // Required default zero-initialization for LLVM globals
        if (var_type->isIntegerTy()) {
            g_var->setInitializer(llvm::ConstantInt::get(var_type, 0));
        } else if (var_type->isPointerTy()) {
            auto* ptr_type = llvm::cast<llvm::PointerType>(var_type);
            g_var->setInitializer(llvm::ConstantPointerNull::get(ptr_type));
        } else if (var_type->isStructTy()) {
            auto* struct_type = llvm::cast<llvm::StructType>(var_type);
            g_var->setInitializer(llvm::ConstantStruct::getNullValue(struct_type));
        }
        return std::any{};
    }

    // Local Scope
    llvm::AllocaInst* alloc = Builder.CreateAlloca(var_type, nullptr, a.id);
    named_values[a.id] = alloc;

    return std::any{};
}
std::any CodeGen::visit(struct VarInitNode& a) {
    llvm::Type* var_type = get_LLVM_type(a.dcl->type);

    // Global Scope
    if (!Builder.GetInsertBlock()) {
        Module->getOrInsertGlobal(a.dcl->id, var_type);
        llvm::GlobalVariable* g_var = Module->getNamedGlobal(a.dcl->id);
        g_var->setLinkage(llvm::GlobalValue::CommonLinkage);

        if (!a.val) {
            if (var_type->isIntegerTy()) {
                g_var->setInitializer(llvm::ConstantInt::get(var_type, 0));
            } else if (var_type->isPointerTy()) {
                auto* ptr_type = llvm::cast<llvm::PointerType>(var_type);
                g_var->setInitializer(llvm::ConstantPointerNull::get(ptr_type));
            } else if (var_type->isStructTy()) {
                auto* struct_type = llvm::cast<llvm::StructType>(var_type);
                g_var->setInitializer(llvm::ConstantStruct::getNullValue(struct_type));
            }
            return std::any{};
        }

        // Evaluate the right-hand side. For globals, this MUST return a constant.
        auto* init_val = std::any_cast<llvm::Value*>(a.val->accept(*this));
        if (!init_val) return std::make_any<llvm::Value*>(nullptr);

        if (auto* const_init = llvm::dyn_cast<llvm::Constant>(init_val)) {
            // Constant Casting
            if (var_type->isIntegerTy(32) && const_init->getType()->isIntegerTy(1)) {
                const_init = llvm::ConstantExpr::getZExt(const_init, Builder.getInt32Ty());
            }
            else if (var_type->isIntegerTy(8) && const_init->getType()->isIntegerTy(32)) {
                const_init = llvm::ConstantExpr::getTrunc(const_init, Builder.getInt8Ty());
            }
            g_var->setInitializer(const_init);
        } else {
            throw std::runtime_error{"ERROR: Global variable '" + a.dcl->id + "' must be initialized with a compile-time constant."};
        }

        return std::any{};
    }

    // Local Scope
    llvm::AllocaInst* alloc = Builder.CreateAlloca(var_type, nullptr, a.dcl->id);
    named_values.insert({a.dcl->id, alloc});

    if (!a.val) {
        return std::any{};
    }

    auto* init_val = std::any_cast<llvm::Value*>(a.val->accept(*this));
    if (!init_val) return std::make_any<llvm::Value*>(nullptr);

    // Casting
    if (var_type->isIntegerTy(32) && init_val->getType()->isIntegerTy(1)) {
        init_val = Builder.CreateZExt(init_val, Builder.getInt32Ty(), "bool_cast_tmp");
    }
    else if (var_type->isIntegerTy(8) && init_val->getType()->isIntegerTy(32)) {
        init_val = Builder.CreateTrunc(init_val, Builder.getInt8Ty(), "char_trunc_tmp");
    }

    Builder.CreateStore(init_val, alloc);

    return std::any{};
}
std::any CodeGen::visit(struct IfNode& a) {
    llvm::Function* the_function = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(*Context, "if_merge");

    for (auto it = a.clauses.begin(); it != a.clauses.end(); ++it) {
        bool is_last_clause = (std::next(it) == a.clauses.end());

        if (it->cond == nullptr) { // i.e. ELSE
            it->block->accept(*this);
            if (!Builder.GetInsertBlock()->getTerminator()) {
                Builder.CreateBr(merge_bb);
            }
            break;
        }

        // Evaluate condition
        auto* cond_val = std::any_cast<llvm::Value*>(it->cond->accept(*this));
        if (!cond_val) return std::make_any<llvm::Value*>(nullptr);

        if (cond_val->getType()->isIntegerTy(32)) { // INT
            cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt32(0), "if_cond");
        }
        else if (cond_val->getType()->isIntegerTy(8)) { // CHAR
            cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt8(0), "if_cond");
        }

        // Create block for code in this clause
        llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(*Context, "then", the_function);

        // Create fallback block (either next clause, or MERGE if done)
        llvm::BasicBlock* next_bb = is_last_clause ? merge_bb : llvm::BasicBlock::Create(*Context, "elif_fallthrough");

        // Branch to 'Then' if true, branch to 'Next' if false
        Builder.CreateCondBr(cond_val, then_bb, next_bb);

        Builder.SetInsertPoint(then_bb);
        it->block->accept(*this);

        // If 'Then' block didn't hit explicit 'return' or 'break', jump to merge
        if (!Builder.GetInsertBlock()->getTerminator()) {
            Builder.CreateBr(merge_bb);
        }

        // Next block
        if (!is_last_clause) {
            the_function->getBasicBlockList().push_back(next_bb);
            Builder.SetInsertPoint(next_bb);
        }
    }

    the_function->getBasicBlockList().push_back(merge_bb);

    Builder.SetInsertPoint(merge_bb);

    return std::any{};
}
std::any CodeGen::visit(struct DeleteNode& a) {
    auto* ptr_to_free = std::any_cast<llvm::Value*>(a.ptr->accept(*this));
    if (!ptr_to_free) return std::make_any<llvm::Value*>(nullptr);

    // Generates: %cast_tmp = bitcast %struct.Vector3* %ptr_to_free to i8*
    llvm::Value* cast_ptr = Builder.CreateBitCast(ptr_to_free, Builder.getInt8PtrTy(), "free_cast_tmp");

    Builder.CreateCall(free_func, cast_ptr);

    return std::any{};
}
std::any CodeGen::visit(struct PrintNode& a) {
    auto* val = std::any_cast<llvm::Value*>(a.args->args.front()->accept(*this));
    if (!val) return std::make_any<llvm::Value*>(nullptr);

    llvm::Function* printf_func = Module->getFunction("printf");
    if (!printf_func) {
        llvm::FunctionType* printf_type = llvm::FunctionType::get(
                Builder.getInt32Ty(), {Builder.getInt8PtrTy()}, true);
        printf_func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage, "printf", Module.get());
    }

    llvm::Type* val_type = val->getType();

    if (val_type->isIntegerTy(32)) { // INT
        llvm::Value* format_ptr = Builder.CreateGlobalStringPtr("%d\n", "fmt_int");
        Builder.CreateCall(printf_func, {format_ptr, val}, "printf_call_int");
    }
    else if (val_type->isIntegerTy(8)) { // CHAR
        llvm::Value* format_ptr = Builder.CreateGlobalStringPtr("%c\n", "fmt_char");
        Builder.CreateCall(printf_func, {format_ptr, val}, "printf_call_char");
    }
    else if (val_type->isIntegerTy(1)) { // BOOL
        llvm::Value* true_str = Builder.CreateGlobalStringPtr("true\n", "str_true");
        llvm::Value* false_str = Builder.CreateGlobalStringPtr("false\n", "str_false");

        llvm::Value* selected_str = Builder.CreateSelect(val, true_str, false_str, "bool_select");

        llvm::Value* format_ptr = Builder.CreateGlobalStringPtr("%s", "fmt_string");
        Builder.CreateCall(printf_func, {format_ptr, selected_str}, "printf_call_bool");
    }

    return std::any{};
}
std::any CodeGen::visit(struct ReturnNode& a) {
    if (a.expr) {
        auto* ret_val = std::any_cast<llvm::Value*>(a.expr->accept(*this));
        Builder.CreateRet(ret_val);
    }
    else {
        Builder.CreateRetVoid();
    }

    return std::any{};
}
std::any CodeGen::visit(struct WhileNode& a) {
    llvm::Function* the_function = Builder.GetInsertBlock()->getParent();

    // 1. Create the blocks
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*Context, "while_cond", the_function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*Context, "while_body", the_function);
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(*Context, "while_after", the_function);

    // 2. Jump from current block into the condition block
    Builder.CreateBr(cond_bb);
    Builder.SetInsertPoint(cond_bb);

    // 3. Evaluate the condition
    auto* cond_val = std::any_cast<llvm::Value*>(a.condition->accept(*this));
    if (!cond_val) return std::make_any<llvm::Value*>(nullptr);

    // (Cast condition to 1-bit boolean if it's an int/char)
    if (cond_val->getType()->isIntegerTy(32)) {
        cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt32(0), "cond_cast");
    } else if (cond_val->getType()->isIntegerTy(8)) {
        cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt8(0), "cond_cast");
    }

    // 4. Branch to body if true, to after if false
    Builder.CreateCondBr(cond_val, body_bb, after_bb);

    // 5. Generate the body
    Builder.SetInsertPoint(body_bb);

    // Push the 'after' block so 'break' nodes know where to go!
    loop_end_blocks.push(after_bb);

    a.statements->accept(*this);

    loop_end_blocks.pop(); // We are done with this loop's scope

    // 6. Loop back to the condition (if the body didn't 'break' or 'return')
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(cond_bb);
    }

    // 7. Continue generating code after the loop
    Builder.SetInsertPoint(after_bb);

    return std::any{};
}
std::any CodeGen::visit(struct AssignmentNode& a) {
    auto* rhs_val = std::any_cast<llvm::Value*>(a.RHS->accept(*this));
    if (!rhs_val) return std::make_any<llvm::Value*>(nullptr);

    llvm::Value* lhs_address = get_LValue(a.LHS.get());
    if (!lhs_address) return std::make_any<llvm::Value*>(nullptr);

    llvm::Type* expected_type = get_LLVM_type(a.LHS->type);

    // Casting
    if (expected_type->isIntegerTy(32) && rhs_val->getType()->isIntegerTy(1)) {
        // Zero-extend the 1-bit true/false into a 32-bit 1 or 0
        rhs_val = Builder.CreateZExt(rhs_val, Builder.getInt32Ty(), "bool_cast_tmp");
    }
    else if (expected_type->isIntegerTy(8) && rhs_val->getType()->isIntegerTy(32)) {
        rhs_val = Builder.CreateTrunc(rhs_val, Builder.getInt8Ty(), "char_trunc_tmp");
    }

    Builder.CreateStore(rhs_val, lhs_address);
    return std::make_any<llvm::Value*>(rhs_val);
}
std::any CodeGen::visit(struct ForNode& a) {
    auto old_named_values = named_values;

    if (a.prologue) a.prologue->accept(*this);

    llvm::Function* the_function = Builder.GetInsertBlock()->getParent();

    // 1. Create blocks
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*Context, "for_cond", the_function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*Context, "for_body", the_function);
    llvm::BasicBlock* inc_bb = llvm::BasicBlock::Create(*Context, "for_inc", the_function);
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(*Context, "for_after", the_function);

    // 2. Jump to condition
    Builder.CreateBr(cond_bb);
    Builder.SetInsertPoint(cond_bb);

    // 3. Evaluate condition (if it exists, e.g., i < 10)
    if (a.cond) {
        auto* cond_val = std::any_cast<llvm::Value*>(a.cond->accept(*this));

        // (Cast condition to 1-bit boolean if needed, same as While loop)
        if (cond_val->getType()->isIntegerTy(32)) {
            cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt32(0));
        }
        else if (cond_val->getType()->isIntegerTy(8)) {
            cond_val = Builder.CreateICmpNE(cond_val, Builder.getInt8(0));
        }

        Builder.CreateCondBr(cond_val, body_bb, after_bb);
    } else {
        // If there's no condition (e.g., for(;;)), it's an infinite loop
        Builder.CreateBr(body_bb);
    }

    // 4. Generate Body
    Builder.SetInsertPoint(body_bb);

    loop_end_blocks.push(after_bb);
    a.block->accept(*this);
    loop_end_blocks.pop();

    // 5. Jump to increment block from the body (if body didn't break/return)
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(inc_bb);
    }

    // 6. Generate Increment (e.g., i++)
    Builder.SetInsertPoint(inc_bb);
    if (a.epilogue) {
        a.epilogue->accept(*this);
    }
    // Loop back to condition
    Builder.CreateBr(cond_bb);

    // 7. Continue after loop
    Builder.SetInsertPoint(after_bb);

    named_values = old_named_values;

    return std::any{};
}
std::any CodeGen::visit(struct BreakNode& a) {
    if (loop_end_blocks.empty())
        throw std::runtime_error{"ERROR: 'break' statement found outside of a loop!"};

    // Jump to the end of the current loop
    llvm::BasicBlock* merge_block = loop_end_blocks.top();
    Builder.CreateBr(merge_block);

    return std::any{};
}
std::any CodeGen::visit(struct NumNode& a) {
    return std::make_any<llvm::Value*>(Builder.getInt32(a.val));
}
std::any CodeGen::visit(struct CharNode& a) {
    return std::make_any<llvm::Value*>(Builder.getInt8(a.val));
}
std::any CodeGen::visit(struct TrueNode& a) {
    return std::make_any<llvm::Value*>(Builder.getInt1(true));
}
std::any CodeGen::visit(struct FalseNode& a) {
    return std::make_any<llvm::Value*>(Builder.getInt1(false));
}
std::any CodeGen::visit(struct IDNode& a) {
    llvm::Value* addr = get_LValue(&a);
    if (!addr)
        throw std::runtime_error{"ERROR: Undefined variable " + a.name};

    llvm::Type* type = nullptr;
    if (auto* alloc = llvm::dyn_cast<llvm::AllocaInst>(addr)) {
        type = alloc->getAllocatedType();
    } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(addr)) {
        type = global->getValueType();
    } else if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(addr)) {
        type = gep->getResultElementType();
    }

    if (!type) {
        throw std::runtime_error{"ERROR: Could not resolve target type for variable load."};
    }

    return std::make_any<llvm::Value*>(
            Builder.CreateLoad(type, addr, a.name + "_load_tmp")
    );
}
std::any CodeGen::visit(struct NilNode& a) {
    return std::make_any<llvm::Value*>(llvm::ConstantPointerNull::get(Builder.getInt8PtrTy()));
}
std::any CodeGen::visit(struct BinaryExprNode& a) {
    // Short-circuiting bools
    if (a.op == Parser::ParserSymbol::OR || a.op == Parser::ParserSymbol::AND) {
        llvm::Function* the_function = Builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* rhs_bb = llvm::BasicBlock::Create(*Context, "sc_rhs", the_function);
        llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(*Context, "sc_merge", the_function);

        // Evaluate LHS first
        auto* L = std::any_cast<llvm::Value*>(a.LHS->accept(*this));
        if (!L) return std::make_any<llvm::Value*>(nullptr);

        llvm::Value* l_bool = L;
        if (L->getType()->isIntegerTy(32)) {
            l_bool = Builder.CreateICmpNE(L, Builder.getInt32(0), "lhs_bool");
        } else if (L->getType()->isIntegerTy(8)) {
            l_bool = Builder.CreateICmpNE(L, Builder.getInt8(0), "lhs_bool");
        }

        // Capture the block we are in AFTER evaluating LHS
        // (LHS might be a nested function call that generated its own blocks)
        llvm::BasicBlock* lhs_eval_bb = Builder.GetInsertBlock();

        // Branch based on operator
        if (a.op == Parser::AND) {
            // AND: If false, short-circuit to merge. If true, evaluate RHS.
            Builder.CreateCondBr(l_bool, rhs_bb, merge_bb);
        } else {
            // OR: If true, short-circuit to merge. If false, evaluate RHS.
            Builder.CreateCondBr(l_bool, merge_bb, rhs_bb);
        }

        // Evaluate RHS
        Builder.SetInsertPoint(rhs_bb);
        auto* R = std::any_cast<llvm::Value*>(a.RHS->accept(*this));
        if (!R) return std::make_any<llvm::Value*>(nullptr);

        // Cast RHS to boolean (i1)
        llvm::Value* r_bool = R;
        if (R->getType()->isIntegerTy(32)) {
            r_bool = Builder.CreateICmpNE(R, Builder.getInt32(0), "rhs_bool");
        } else if (R->getType()->isIntegerTy(8)) {
            r_bool = Builder.CreateICmpNE(R, Builder.getInt8(0), "rhs_bool");
        }

        // Capture the block AFTER evaluating RHS, then jump to merge
        llvm::BasicBlock* rhs_eval_bb = Builder.GetInsertBlock();
        Builder.CreateBr(merge_bb);

        // Merge Block
        Builder.SetInsertPoint(merge_bb);
        llvm::PHINode* phi = Builder.CreatePHI(Builder.getInt1Ty(), 2, "sc_result");

        if (a.op == Parser::AND) {
            phi->addIncoming(Builder.getInt1(false), lhs_eval_bb); // Skipped RHS -> result is false
            phi->addIncoming(r_bool, rhs_eval_bb);                 // Evaluated RHS -> result is RHS
        } else {
            phi->addIncoming(Builder.getInt1(true), lhs_eval_bb);  // Skipped RHS -> result is true
            phi->addIncoming(r_bool, rhs_eval_bb);                 // Evaluated RHS -> result is RHS
        }

        // Zero-extend back to 32-bit int so it plays nicely with the rest of your typed expressions
        llvm::Value* final_result = Builder.CreateZExt(phi, Builder.getInt32Ty(), "sc_zext");

        return std::make_any<llvm::Value*>(final_result);
    }

    auto* L = std::any_cast<llvm::Value*>(a.LHS->accept(*this));
    auto* R = std::any_cast<llvm::Value*>(a.RHS->accept(*this));

    if (!L || !R) return std::make_any<llvm::Value*>(nullptr);

    const bool l_is_ptr = is_pointer(a.LHS->type);
    const bool r_is_ptr = is_pointer(a.RHS->type);

    if (!l_is_ptr && !r_is_ptr) {
        if (L->getType()->isIntegerTy(8) && R->getType()->isIntegerTy(32)) {
            L = Builder.CreateSExt(L, Builder.getInt32Ty(), "promoted_L");
        }
        else if (L->getType()->isIntegerTy(32) && R->getType()->isIntegerTy(8)) {
            R = Builder.CreateSExt(R, Builder.getInt32Ty(), "promoted_R");
        }
        else if (L->getType()->isIntegerTy(1) && R->getType()->isIntegerTy(32)) {
            L = Builder.CreateZExt(L, Builder.getInt32Ty(), "promoted_L_bool");
        }
        else if (L->getType()->isIntegerTy(32) && R->getType()->isIntegerTy(1)) {
            R = Builder.CreateZExt(R, Builder.getInt32Ty(), "promoted_R_bool");
        }
    }

    llvm::Value* result = nullptr;
    switch (a.op) {
        case Parser::OR:
            result = Builder.CreateLogicalOr(L, R, "or_tmp");
            break;
        case Parser::AND:
            result = Builder.CreateLogicalAnd(L, R, "and_tmp");
            break;
        case Parser::GEQ:
            result = Builder.CreateICmpSGE(L, R, "geq_tmp");
            break;
        case Parser::GT:
            result = Builder.CreateICmpSGT(L, R, "gt_tmp");
            break;
        case Parser::LEQ:
            result = Builder.CreateICmpSLE(L, R, "leq_tmp");
            break;
        case Parser::LT:
            result = Builder.CreateICmpSLT(L, R, "lt_tmp");
            break;
        case Parser::EQUALS:
            result = Builder.CreateICmpEQ(L, R, "eq_tmp");
            break;
        case Parser::NEQ:
            result = Builder.CreateICmpNE(L, R, "neq_tmp");
            break;
        case Parser::ParserSymbol::PLUS: {
            if (l_is_ptr && !r_is_ptr) {
                if (R->getType()->isIntegerTy(8)) {
                    R = Builder.CreateSExt(R, Builder.getInt32Ty(), "sext_idx");
                }
                std::string base_type = a.LHS->type;
                base_type.pop_back();
                llvm::Type* pointee_type = get_LLVM_type(base_type);

                result = Builder.CreateGEP(pointee_type, L, R, "ptr_add_tmp");
            }
            else if (!l_is_ptr && r_is_ptr) {
                if (L->getType()->isIntegerTy(8)) {
                    L = Builder.CreateSExt(L, Builder.getInt32Ty(), "sext_idx");
                }
                std::string base_type = a.RHS->type;
                base_type.pop_back();
                llvm::Type* pointee_type = get_LLVM_type(base_type);

                result = Builder.CreateGEP(pointee_type, R, L, "ptr_add_tmp");
            }
            else {
                result = Builder.CreateAdd(L, R, "add_tmp");
            }
            break;
        }
        case Parser::SUB: {
            if (l_is_ptr && !r_is_ptr) {
                if (R->getType()->isIntegerTy(8)) {
                    R = Builder.CreateSExt(R, Builder.getInt32Ty(), "sext_idx");
                }
                std::string base_type = a.LHS->type;
                base_type.pop_back();
                llvm::Type* pointee_ty = get_LLVM_type(base_type);

                llvm::Value* diff = Builder.CreatePtrDiff(pointee_ty, L, R, "ptr_diff_tmp");
                result = Builder.CreateTrunc(diff, Builder.getInt32Ty(), "diff_trunc_tmp");
            } else if (l_is_ptr && r_is_ptr) {
                std::string base_type = a.LHS->type;
                base_type.pop_back();
                llvm::Type *pointee_ty = get_LLVM_type(base_type);

                result = Builder.CreatePtrDiff(pointee_ty, L, R, "ptr_diff_tmp");
            } else {
                result = Builder.CreateSub(L, R, "sub_tmp");
            }
            break;
        }
        case Parser::MULT:
            result = Builder.CreateMul(L, R, "mult_tmp");
            break;
        case Parser::DIV:
            result = Builder.CreateSDiv(L, R, "div_tmp");
            break;
        case Parser::MOD:
            result = Builder.CreateSRem(L, R, "mod_tmp");
            break;
        case Parser::LSHIFT:
            result = Builder.CreateShl(L, R, "shl_tmp");
            break;
        case Parser::RSHIFT:
            result = Builder.CreateAShr(L, R, "shr_tmp");
            break;
        case Parser::EXP: {
            llvm::Function* pow_func = Module->getFunction("pow");
            if (!pow_func) {
                llvm::FunctionType* pow_type = llvm::FunctionType::get(
                    Builder.getDoubleTy(), {Builder.getDoubleTy(), Builder.getDoubleTy()}, false
                );
                pow_func = llvm::Function::Create(
                    pow_type, llvm::Function::ExternalLinkage, "pow", Module.get()
                );
            }

            llvm::Value* l_double = Builder.CreateSIToFP(L, Builder.getDoubleTy(), "l_cast");
            llvm::Value* r_double = Builder.CreateSIToFP(R, Builder.getDoubleTy(), "r_cast");

            llvm::Value* pow_result = Builder.CreateCall(pow_func, {l_double, r_double}, "pow_call");

            result = Builder.CreateFPToSI(pow_result, Builder.getInt32Ty(), "pow_cast_back");
            break;
        }
        case Parser::BITOR:
            result = Builder.CreateOr(L, R, "bor_tmp");
            break;
        case Parser::BITXOR:
            result = Builder.CreateXor(L, R, "bxor_tmp");
            break;
        case Parser::BITAND:
            result = Builder.CreateAnd(L, R, "band_tmp");
            break;
        default:
            throw std::runtime_error{"ERROR: Invalid Binary Expression Operator Found"};
    }

    return std::make_any<llvm::Value*>(result);
}
std::any CodeGen::visit(struct MemberAccessExprNode& a) {
    llvm::Value* field_address = get_LValue(&a);
    if (!field_address) return std::make_any<llvm::Value*>(nullptr);

    llvm::Type* field_type = get_LLVM_type(a.type);

    llvm::Value* loaded_data = Builder.CreateLoad(field_type, field_address, "member_load_tmp");

    return std::make_any<llvm::Value*>(loaded_data);
}
std::any CodeGen::visit(struct UnaryExprNode& a) {
    if (a.op == Parser::ParserSymbol::ADDR) {
        llvm::Value* address = get_LValue(a.arg.get());
        if (!address) throw std::runtime_error{"ERROR: Cannot take address of R-Value"};
        return std::make_any<llvm::Value*>(address);
    }

    auto* arg = std::any_cast<llvm::Value*>(a.arg->accept(*this));
    if (!arg) return std::make_any<llvm::Value*>(nullptr);

    llvm::Value* result = nullptr;
    switch (a.op) {
        case Parser::NOT:
            result = Builder.CreateNot(arg, "not_tmp");
            break;
        case Parser::expr3: // PLUS
            result = arg;
            break;
        case Parser::expr4: // SUB
            result = Builder.CreateNeg(arg, "neg_tmp");
            break;
        case Parser::BITNOT:
            result = Builder.CreateNot(arg, "bnot_tmp");
            break;
        case Parser::INCR:
        case Parser::DECR: {
            llvm::Value* target_address = get_LValue(a.arg.get());
            if (!target_address) return std::make_any<llvm::Value*>(nullptr);

            // 1. DYNAMIC TYPE EXTRACTION (Ignores a.type completely)
            llvm::Type* target_type = nullptr;
            if (auto* alloc = llvm::dyn_cast<llvm::AllocaInst>(target_address)) {
                target_type = alloc->getAllocatedType();
            } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(target_address)) {
                target_type = global->getValueType();
            } else if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(target_address)) {
                target_type = gep->getResultElementType();
            }

            if (!target_type) {
                throw std::runtime_error{"ERROR: Could not resolve target type for INCR/DECR."};
            }

            // 2. Load the current value using the extracted type
            llvm::Value* current_value = Builder.CreateLoad(target_type, target_address, "load_tmp");

            // 3. Generate the increment/decrement based on LLVM type
            if (target_type->isIntegerTy(32)) { // INT
                llvm::Value* one = Builder.getInt32(1);
                if (a.op == Parser::ParserSymbol::INCR) {
                    result = Builder.CreateAdd(current_value, one, "incr_tmp");
                } else {
                    result = Builder.CreateSub(current_value, one, "decr_tmp");
                }
            }
            else if (target_type->isIntegerTy(8)) { // CHAR
                llvm::Value* one = Builder.getInt8(1);
                if (a.op == Parser::ParserSymbol::INCR) {
                    result = Builder.CreateAdd(current_value, one, "incr_char_tmp");
                } else {
                    result = Builder.CreateSub(current_value, one, "decr_char_tmp");
                }
            }
            else if (target_type->isPointerTy()) { // POINTERS
                llvm::Value* offset = Builder.getInt32(a.op == Parser::INCR ? 1 : -1);
                result = Builder.CreateGEP(target_type, current_value, offset, "ptr_inc");
            } else {
                throw std::runtime_error{"ERROR: Cannot increment/decrement this type."};
            }

            Builder.CreateStore(result, target_address);
            break;
        }
        case Parser::AT: {
            llvm::Value* ptr_val = arg;
            llvm::Type* pointee_type = get_LLVM_type(a.type);

            result = Builder.CreateLoad(pointee_type, ptr_val, "deref_tmp");

            break;
        }
        case Parser::ADDR: {
            llvm::Value *address = get_LValue(a.arg.get());
            if (!address)
                throw std::runtime_error{"ERROR: Cannot take address of R-Value"};
            result = address;
            break;
        }
        default:
            throw std::runtime_error{"ERROR: Invalid Unary Expression Operator Found"};
    }

    return std::make_any<llvm::Value*>(result);
}
std::any CodeGen::visit(struct AllocNode& a) {
    llvm::Type* alloc_type = get_LLVM_type(a.type);
    if (!alloc_type) return std::make_any<llvm::Value*>(nullptr);

    uint64_t type_size = Module->getDataLayout().getTypeAllocSize(alloc_type);
    llvm::Value* size_val = Builder.getInt64(type_size);

    // Generates: %malloc_tmp = call i8* @malloc(i64 12)
    llvm::Value* allocated_memory = Builder.CreateCall(malloc_func, size_val, "malloc_tmp");

    return std::make_any<llvm::Value*>(allocated_memory);
}
std::any CodeGen::visit(struct FunctionCallNode& a) {
    llvm::Function* callee = Module->getFunction(a.id);
    if (!callee)
        throw std::runtime_error{"ERROR: Unknown function referenced: " + a.id};

    std::vector<llvm::Value*> args_v;

    if (a.args) {
        for (auto& arg_expr : a.args->args) {
            auto* val = std::any_cast<llvm::Value*>(arg_expr->accept(*this));
            if (!val) return std::make_any<llvm::Value*>(nullptr);
            args_v.push_back(val);
        }
    }

    if (callee->getReturnType()->isVoidTy()) {
        return std::make_any<llvm::Value*>(Builder.CreateCall(callee, args_v));
    }
    else {
        return std::make_any<llvm::Value*>(Builder.CreateCall(callee, args_v, "call_tmp"));
    }
}
std::any CodeGen::visit(struct ReadCallNode& a) {
    llvm::Function* scanf_func = Module->getFunction("scanf");
    if (!scanf_func) {
        llvm::FunctionType* scanf_type = llvm::FunctionType::get(
                Builder.getInt32Ty(), {Builder.getInt8PtrTy()}, true);
        scanf_func = llvm::Function::Create(
                scanf_type, llvm::Function::ExternalLinkage, "scanf", Module.get());
    }

    llvm::Value* format_ptr = Builder.CreateGlobalStringPtr("%c", "read_char_fmt");

    llvm::AllocaInst* temp_alloc = Builder.CreateAlloca(Builder.getInt8Ty(), nullptr, "read_char_tmp");

    Builder.CreateStore(Builder.getInt8(0), temp_alloc);

    Builder.CreateCall(scanf_func, {format_ptr, temp_alloc}, "scanf_call");

    llvm::Value* result = Builder.CreateLoad(Builder.getInt8Ty(), temp_alloc, "read_char_val");

    return std::make_any<llvm::Value*>(result);
}
