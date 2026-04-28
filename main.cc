#include <fstream>
#include <iostream>
#include "scanner/scanner.h"

int main(int argc, char* argv[]) {
    std::ifstream ifs{"../xer/sample_program.xer"};
    // std::ofstream ofs{"../xer/sample_program.tokens"};
    std::ofstream ofs{"/dev/null"};
    std::vector<Token> stream;
    scan(ifs, ofs, stream, std::cerr);
}