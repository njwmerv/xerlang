#ifndef TYPES_H
#define TYPES_H

#define MAX_RHS_LEN 11

namespace Scanner {
    enum ScannerDFAState {
        // ACCEPTINGs
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT, VOID,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-ACCEPTINGs
        START, APOS, APOSLASH, NOT_CHARLIT,
        ERROR, NUM_STATES,
    };
}

std::ostream& operator<<(std::ostream& os, Scanner::ScannerDFAState state);

namespace Parser {
    enum ParserSymbol {
        // TERMINATING
        MAIN, READ, PRINT, INT, CHAR, BOOL, STRUCT, VOID,
        TRUE, FALSE, NIL, NUM, CHARLIT, ID,
        RETURN, IF, ELIF, ELSE, FOR, WHILE, BREAK,
        DELETE, NEW,
        COLON, LPAREN, RPAREN, SEMI, LCURLY, RCURLY, COMMA, LBRACK, RBRACK, BECOMES,
        NOT, OR, AND, GEQ, GT, LEQ, LT, EQUALS, NEQ,
        PLUS, SUB, MULT, DIV, MOD, LSHIFT, RSHIFT, EXP,
        BITOR, BITXOR, BITAND, BITNOT,
        AT, ADDR, INCR, DECR, ARROW, DOT,

        // NON-TERMINATING
        BoF, EoF, start,
        procedures, procedure, main,
        type, star,
        params, paramlist, dcls, dcl,
        statements, statement, ifs, forprologue, forepilogue,
        expr1, expr2, expr3, expr4, expr5, expr6, expr7, expr8, expr9, expr10, expr11, expr12, expr13, expr14,
        args, structdef,
        DOLLAR, NUM_SYMBOLS,
    };
}

std::ostream& operator<<(std::ostream& os, Parser::ParserSymbol state);

typedef struct {
    std::string lexeme;
    Parser::ParserSymbol type;
    size_t line_num;
    size_t col_num;
} Token;

struct Production {
    Parser::ParserSymbol LHS;
    size_t len;
    std::array<Parser::ParserSymbol, MAX_RHS_LEN> RHS;
};

struct ASTNode {};

#endif //TYPES_H
