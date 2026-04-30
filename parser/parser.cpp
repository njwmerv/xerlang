#include "parser.h"
#include <optional>
#include <vector>
#include <memory>

struct Production {
    const Parser::ParserSymbol LHS;
    const std::vector<Parser::ParserSymbol> RHS;
};

struct ASTNode {
    ASTNode* parent;
    Production production;
    std::optional<Type> type;
    std::vector<std::unique_ptr<ASTNode*>> children;

    ~ASTNode() { parent = nullptr; }
};

struct Procedure {
    Type return_type;
    std::vector<Type> params;
    std::unordered_map<std::string, Type> symbol_table;
};

std::unordered_map<Type, std::string> TYPES{
    {0, "void"},
    {1, "int"}, {2, "char"}, {3, "bool"},
    {4, "int@"}, {5, "char@"}, {6, "bool@"},
};

std::unordered_map<std::string, Procedure> PROCEDURE_SYMBOL_TABLE{ // RETHINK THIS CAUSE FUNCTIONS CAN TAKE DIFFERENT PARAMS BASED ON INPUT

};


