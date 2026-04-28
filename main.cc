#include <fstream>
#include <iostream>
#include "scanner/scanner.h"

int main() {
    std::ifstream ifs{"../xer/sample_program.xer"};
    std::ofstream ofs{"../xer/sample_program.tokens"};
    std::vector<Token> stream;
    scan(ifs, ofs, stream);

    for (auto& token : stream) {
        std::cout << token.lexeme << std::endl;
    }
}