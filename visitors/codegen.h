#ifndef CODEGEN_H
#define CODEGEN_H

#include <memory>
#include <stack>
#include <map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include "../parser/ast.h"
#include "../util/types.h"

class CodeGen : public Visitor {
private:
    std::unique_ptr<llvm::LLVMContext> Context;
    std::unique_ptr<llvm::Module> Module;
    llvm::IRBuilder<> Builder;

    llvm::FunctionCallee malloc_func;
    llvm::FunctionCallee free_func;

    std::map<std::string, llvm::AllocaInst*> named_values;
    std::map<std::string, llvm::StructType*> struct_types;
    std::map<std::string, std::map<std::string, size_t>> struct_field_indices;
    std::stack<llvm::BasicBlock*> loop_end_blocks;

    llvm::Value* get_LValue(ASTNode* node);

    llvm::Type* get_LLVM_type(const std::string& type);

public:

    CodeGen() :
        Context{std::make_unique<llvm::LLVMContext>()},
        Module{std::make_unique<llvm::Module>("xerlang_compiler", *Context)},
        Builder{*Context},
        malloc_func{Module->getOrInsertFunction("malloc", llvm::FunctionType::get(
            Builder.getInt8PtrTy(), {Builder.getInt64Ty()}, false
        ))},
        free_func{Module->getOrInsertFunction("free", llvm::FunctionType::get(
            Builder.getVoidTy(), {Builder.getInt8PtrTy()}, false
        ))}
        {}

    void print_ir(llvm::raw_fd_ostream&);

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


#endif // CODEGEN_H
