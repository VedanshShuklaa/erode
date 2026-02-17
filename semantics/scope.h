#pragma once
#include <memory>
#include <optional>
#include <unordered_map>
#include "../parser/parser.h"
#include "../token/token.h"

struct Symbol {
    std::string name;
    TypeKind type;
    bool isFunction = false;
    std::vector<TypeKind> params;
};

class Scope {
public:
    Scope* parent;
    std::unordered_map<std::string, Symbol> symbols;

    Scope() : parent(nullptr) {}
    Scope(Scope* t_parent) : parent(t_parent) {}

    bool insert(const std::string& t_name, const Symbol& t_symbol) {
        return symbols.emplace(t_name, t_symbol).second;
    }

    std::optional<Symbol> lookup(const std::string& t_name) {
        if (symbols.count(t_name)) {
            return symbols[t_name];
        }
        if (parent) {
            return parent->lookup(t_name);
        }
        return std::nullopt;
    }
};