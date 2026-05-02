#include <fstream>
#include <iostream>
#include <vector>
#include "parser/parser.h"
#include "scanner/scanner.h"
#include "util/types.h"

int main(int argc, char* argv[]) {
    // Scanner
    std::ifstream ifs{"../xer/sample_program.xer"};
    std::ofstream ofs{"../xer/sample_program.tokens"}; // std::ofstream ofs{"/dev/null"};
    std::vector<Token> stream = {{{}, Parser::ParserSymbol::BoF}};
    scan(ifs, ofs, stream, std::cerr);
    stream.push_back({{}, Parser::ParserSymbol::EoF});
    stream.push_back({{}, Parser::ParserSymbol::DOLLAR});

    // Parser
    std::unique_ptr<ASTNode> root;
    try {
        root = parse(stream);
        print_AST(root, 0, std::cout);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
    }
}