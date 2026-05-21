#include "symbol_table.h"
#include <ranges>

SymbolTable::SymbolTable() {
    enter_scope();
}

const std::vector<SymbolTableEntry>& SymbolTable::see_syms() const { return all_syms; }

void SymbolTable::enter_scope() {
    scopes.emplace_back();
}

void SymbolTable::exit_scope() {
    if (!scopes.empty()) scopes.pop_back();
}

void SymbolTable::declare_variable(const std::string& id, const std::string& type, int size) {
    auto& current_scope = scopes.back();

    if (current_scope.find(id) != current_scope.end())
        throw std::runtime_error{"ERROR: Redefining variable: " + id};

    current_stack_offset -= size;

    current_scope.insert({id, {.id = id, .type = type, .frame_offset = current_stack_offset}});
    all_syms.push_back({.id = id, .type = type, .frame_offset = current_stack_offset});
}

SymbolTableEntry* SymbolTable::lookup_variable(const std::string& name) {
    for (auto& current_scope : std::ranges::reverse_view(scopes)) {
        if (current_scope.find(name) != current_scope.end())
            return &current_scope.at(name);
    }
    return nullptr;
}

void SymbolTable::parameterize() {
    const int max_offset = -current_stack_offset;
    for (auto& [id, ste] : scopes.back()) {
        ste.frame_offset += max_offset;
    }
    for (auto& ste : all_syms) {
        ste.frame_offset += max_offset;
    }
    current_stack_offset = 0;
}
