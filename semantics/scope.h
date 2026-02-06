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
    Scope(Scope* parent) : parent(parent) {}

    bool insert(const std::string& name, const Symbol& symbol) {
        return symbols.emplace(name, symbol).second;
    }

    std::optional<Symbol> lookup(const std::string& name) {
        if (symbols.count(name)) {
            return symbols[name];
        }
        if (parent) {
            return parent->lookup(name);
        }
        return std::nullopt;
    }
};

