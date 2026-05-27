#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include "parser/parser.h"
#include "scanner/scanner.h"
#include "util/types.h"
#include "visitors/printer.h"
#include "visitors/typecheck.h"

#define SCAN_ONLY       1 << 0
#define PARSE_ONLY      1 << 1
#define SEMANTIC_ONLY   1 << 2
#define IR_ONLY         1 << 3
#define NO_OPTIMIZE     1 << 4
#define CODE_ONLY       1 << 5

int main(int argc, char* argv[]) {
    size_t flags = 0;
    std::string file;
    std::string exec_name;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-sc") == 0) {
            flags |= SCAN_ONLY;
            break;
        }
        else if (strcmp(argv[i], "-pa") == 0) {
            flags |= PARSE_ONLY;
            break;
        }
        else if (strcmp(argv[i], "-sa") == 0) {
            flags |= SEMANTIC_ONLY;
            break;
        }
        else if (strcmp(argv[i], "-ir") == 0) {
            flags |= IR_ONLY;
            break;
        }
        else if (strcmp(argv[i], "-no") == 0) {
            flags |= NO_OPTIMIZE;
        }
        else if (strcmp(argv[i], "-co") == 0) {
            flags |= CODE_ONLY;
            break;
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
    if (file.empty()) {
        std::cerr << "ERROR: Never given file to compile\n";
        return 4;
    }

    // Scanner
    std::vector<Token> stream = {{{}, Parser::ParserSymbol::BoF}};
    try {
        std::ifstream ifs{file};
        std::ofstream ofs{"/dev/null"};
        scan(ifs, ofs, stream, std::cerr);
        if (stream.back().type == Parser::ParserSymbol::DOLLAR) return 1;
        stream.push_back({{}, Parser::ParserSymbol::EoF});
        stream.push_back({{}, Parser::ParserSymbol::DOLLAR});
    }
    catch (std::exception& e) {
        std::cerr << "ERROR: Scanner: " << e.what() << '\n';
        return 4;
    }

    if (flags & SCAN_ONLY) {
        for (const auto& token : stream) {
            std::cout << token.type << " : " << token.lexeme << " : (" << token.line_num << ", " << token.col_num << ")\n";
        }
        return 0;
    }

    // Parser
    std::unique_ptr<ASTNode> root = parse(stream, std::cerr);
    if (flags & PARSE_ONLY) {
        Printer printer;
        root->accept(printer);
        return 0;
    }

    // Semantic Analysis
    TypeChecker tc;
    root->accept(tc);

    if (flags & SEMANTIC_ONLY) {
        Printer printer;
        root->accept(printer);
        return 0;
    }

    // Convert to IR


    if (flags & IR_ONLY) {
        // PRINT IR
        return 0;
    }

    // Optimize
    if (!(flags & NO_OPTIMIZE)) {

    }

    // Code Generation

    if (!(flags & CODE_ONLY)) {
        // Ignore headers that make it a valid executable
    }

    return 0;
}