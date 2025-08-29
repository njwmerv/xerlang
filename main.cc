#include <iostream>
#include <string>
#include "scanner.h"

int main() {
    const Scanner* scanner = Scanner::getInstance();

    const std::string fileNameStem = "../sample_program";
    scanner->scan(fileNameStem);
}