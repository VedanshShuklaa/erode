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

    FunctionDef(std::string t_name, std::vector<Param> t_params, BlockStmt* t_body, TypeKind t_returnType)
        : name(t_name), params(t_params), body(t_body), returnType(t_returnType) {}
};

struct ExternDecl : Item {
    std::string name;
    std::vector<Param> params;
    TypeKind returnType;
    
    ExternDecl(std::string t_name, std::vector<Param> t_params, TypeKind t_returnType)
        : name(t_name), params(t_params), returnType(t_returnType) {}
};
