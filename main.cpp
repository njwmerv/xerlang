#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "parser/parser.h"
#include "scanner/scanner.h"
#include "util/types.h"
#include "visitors/printer.h"
#include "visitors/typecheck.h"
#include "visitors/codegen.h"

#define SCAN_ONLY       1 << 0
#define PARSE_ONLY      1 << 1
#define SEMANTIC_ONLY   1 << 2
#define IR_ONLY         1 << 3
#define NO_OPTIMIZE     1 << 4
#define CODE_ONLY       1 << 5

void generate_executable(llvm::Module* module, const std::string& output_file) {
    // 1. Initialize targets
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    // 2. Set up the target machine
    std::string error;
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
    auto target_machine = target->createTargetMachine(target_triple, "generic", "", opt, RM);

    // 3. Set up the pass manager to emit an object file
    module->setDataLayout(target_machine->createDataLayout());
    module->setTargetTriple(target_triple);

    std::error_code ec;
    llvm::raw_fd_ostream dest(output_file, ec, llvm::sys::fs::OF_None);

    llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();
}

void link_executable(const std::string& obj_file, const std::string& exec_name) {
    // Construct the command to call the linker (e.g., clang or gcc)
    std::string command = "clang " + obj_file + " -o " + exec_name;

    // Execute the command
    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Linking failed!");
    }
}

int main(int argc, char* argv[]) {
    auto start = std::chrono::steady_clock::now();

    size_t flags = 0;
    std::string file;
    std::string exec_name;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sc") == 0) {
            flags |= SCAN_ONLY;
        }
        else if (strcmp(argv[i], "-pa") == 0) {
            flags |= PARSE_ONLY;
        }
        else if (strcmp(argv[i], "-sa") == 0) {
            flags |= SEMANTIC_ONLY;
        }
        else if (strcmp(argv[i], "-ir") == 0) {
            flags |= IR_ONLY;
        }
        else if (strcmp(argv[i], "-no") == 0) {
            flags |= NO_OPTIMIZE;
        }
        else if (strcmp(argv[i], "-co") == 0) {
            flags |= CODE_ONLY;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (!exec_name.empty()) {
                std::cerr << "ERROR: Given 2 executable names\n";
                return 1;
            }
            if (i >= argc - 1 || argv[i+1][0] == '-') {
                std::cerr << "ERROR: Expected executable name after -o flag\n";
                return 2;
            }
            exec_name = argv[++i];
        }
        else {
            if (!file.empty()) {
                std::cerr << "ERROR: Given multiple files to compile\n";
                return 3;
            }
            file = argv[i];
        }
    }
//    file = "../xer/sample_program.xer";
    file = "../xer/hello_world.xer";
    if (file.empty()) {
        std::cerr << "ERROR: Never given file to compile\n";
        return 4;
    }

    std::string file_root;
    if (file.ends_with(".xer")) file_root = file.substr(0, file.size() - 4);
    else file_root = file;

    // Scanner
    std::vector<Token> stream = {{{}, Parser::ParserSymbol::BoF}};
    try {
        std::ifstream ifs{file};
        scan(ifs, stream, std::cerr);
        if (stream.back().type == Parser::ParserSymbol::DOLLAR) return 1;
        stream.push_back({{}, Parser::ParserSymbol::EoF});
        stream.push_back({{}, Parser::ParserSymbol::DOLLAR});
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Scanner: " << e.what() << '\n';
        return 4;
    }

    if (flags & SCAN_ONLY) {
        std::ofstream xert{file_root + ".xert"};
        for (const auto& token : stream) {
            xert << token.type << " : " << token.lexeme << " : (" << token.line_num << ", " << token.col_num << ")\n";
        }
        return 0;
    }

    // Parser
    std::unique_ptr<ASTNode> root;
    try {
        root = parse(stream, std::cerr);
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Parser: " << e.what() << '\n';
        return 5;
    }
    if (flags & PARSE_ONLY) {
        std::ofstream xerp{file_root + ".xerp"};
        Printer printer{xerp};
        root->accept(printer);
        return 0;
    }

    // Semantic Analysis
    try {
        TypeChecker tc;
        root->accept(tc);
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Typechecker: " << e.what() << '\n';
        return 6;
    }
    if (flags & SEMANTIC_ONLY) {
        std::ofstream xerp{file_root + ".xerp"};
        Printer printer{xerp};
        root->accept(printer);
        return 0;
    }

    // Convert to IR

    CodeGen cg{};
    try {
        root->accept(cg);
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: IR Generator: " << e.what() << '\n';
        return 7;
    }

    // Optimize
    if (!(flags & NO_OPTIMIZE)) {
        cg.optimize();
    }

    std::string ll_file = file_root + ".ll";
    std::error_code error_code;
    llvm::raw_fd_ostream ll{ll_file, error_code, llvm::sys::fs::OF_None};
    if (error_code) {
        llvm::errs() << "ERROR: Could not open file: " << error_code.message() << "\n";
        return 8;
    }
    cg.print_ir(ll);
    if (flags & IR_ONLY) return 0;

    // Code Generation

    std::string obj_file = file_root + ".o";
    generate_executable(cg.get_module(), obj_file);

    if (!(flags & CODE_ONLY)) {
        if (exec_name.empty()) exec_name = file_root; // Default name
        link_executable(obj_file, exec_name);

        std::remove(obj_file.c_str());
        std::remove(ll_file.c_str());
    }

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;

    std::cout << "Elapsed time: " << (elapsed_seconds.count() * 1000) << " ms\n";

    return 0;
}