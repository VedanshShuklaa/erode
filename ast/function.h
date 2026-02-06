//Header file for all the function related AST nodes

#pragma once
#include "item.h"
#include "statement.h"

struct Param {
    TypeKind type;
    std::string name;
};

struct FunctionDef : Item {
    std::string name;
    std::vector<Param> params;
    BlockStmt* body;
    TypeKind returnType;

    FunctionDef(std::string name, std::vector<Param> params, BlockStmt* body, TypeKind returnType)
        : name(name), params(params), body(body), returnType(returnType) {}
};

struct ExternDecl : Item {
    std::string name;
    std::vector<Param> params;
    TypeKind returnType;
    
    ExternDecl(std::string name, std::vector<Param> params, TypeKind returnType)
        : name(name), params(params), returnType(returnType) {}
};
