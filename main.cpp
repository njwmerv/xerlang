#include <iostream>
#include <fstream>
#include <vector>
#include "scanner/scanner.h"
#include "util/types.h"

int main(int argc, char* argv[]) {
    // Scanner
    std::ifstream ifs{"../xer/sample_program.xer"};
    std::ofstream ofs{"../xer/sample_program.tokens"}; // std::ofstream ofs{"/dev/null"};
    std::vector<Token> stream = {{{}, Parser::ParserSymbol::BoF}};
    scan(ifs, ofs, stream, std::cerr);
}