#include <iostream>
#include <string>
#include "scanner/scanner.h"

int main() {
    const Scanner* scanner = Scanner::getInstance();

    const std::string fileNameStem = "../xer/sample_program";
    scanner->scan(fileNameStem);
}