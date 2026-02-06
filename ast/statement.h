//Statement header file for all the statement related AST nodes

#pragma once
#include "item.h"
#include "expression.h"

struct Statement : Item {
    
};

struct ExprStmt : Statement {
    Expression* expr;

    ExprStmt(Expression* expr) : expr(expr) {}
};

struct VarDeclStmt : Statement {
    TypeKind kind;
    std::string name;
    Expression* initializer;

    VarDeclStmt(TypeKind kind, std::string name, Expression* initializer)
        : kind(kind), name(name), initializer(initializer) {}
};

struct BlockStmt : Statement {
    std::vector<Statement*> statements;
};

struct ReturnStmt : Statement {
    Expression* value;

    ReturnStmt(Expression* value) : value(value) {}
};