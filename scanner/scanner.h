#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>

namespace Scanner {
    enum ScannerDFAState {
        // NON-ACCEPTINGs
        START, APOS, APOSLASH, NOT_CHARLIT,
        // ACCEPTINGs
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        // ! || && >= > > <= < == !=
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        // + - * / % << >> ^ | & ~
        AT, ADDR, INCR, DECR, ARROW, DOT,
        // ++ -- -> .
        ERROR, NUM_STATES,
    };
}

std::ostream& operator<<(std::ostream& os, Scanner::ScannerDFAState state);

typedef struct Token {
    std::string lexeme;
    Scanner::ScannerDFAState type;
} Token;

void scan(std::istream& is, std::ostream& os, std::vector<Token>& stream, std::ostream& err);

#endif //SCANNER_H
