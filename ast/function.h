#pragma once
#include "item.h"
#include "statement.h"
#include <optional>

struct Param {
    TypeKind type;
    std::string name;
};

struct FunctionDef : Item {
    std::string name;
    std::vector<Param> params;
    BlockStmt* body;
    std::optional<TypeKind> returnType;

    FunctionDef(std::string name, std::vector<Param> params, BlockStmt* body, std::optional<TypeKind> returnType)
        : name(name), params(params), body(body), returnType(returnType) {}
};

struct ExternDecl : Item {
    std::string name;
    std::vector<Param> params;
    
    ExternDecl(std::string name, std::vector<Param> params)
        : name(name), params(params) {}
};
