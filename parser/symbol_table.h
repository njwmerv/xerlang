#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "util/types.h"

class SymbolTable {
    int current_stack_offset = 0;
    std::vector<std::unordered_map<std::string, SymbolTableEntry>> scopes;
    std::vector<SymbolTableEntry> all_syms;

public:
    SymbolTable();

    [[nodiscard]] const std::vector<SymbolTableEntry>& see_syms() const;

    void enter_scope();
    void exit_scope();
    void declare_variable(const std::string& id, const std::string& type, int size);
    SymbolTableEntry* lookup_variable(const std::string& name);

    void parameterize();
};

#endif // SYMBOLTABLE_H
