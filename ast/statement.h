#pragma once
#include "item.h"
#include "expression.h"

struct Statement : Item {
    
};

struct ExprStmt : Statement {
    Expression* expr;

    ExprStmt(Expression* t_expr) : expr(t_expr) {}
};

struct VarDeclStmt : Statement {
    TypeKind kind;
    std::string name;
    Expression* initializer;

    VarDeclStmt(TypeKind t_kind, std::string t_name, Expression* t_initializer)
        : kind(t_kind), name(t_name), initializer(t_initializer) {}
};

struct BlockStmt : Statement {
    std::vector<Statement*> statements;
};

struct ReturnStmt : Statement {
    Expression* value;

    ReturnStmt(Expression* t_value) : value(t_value) {}
};

struct IfStmt : Statement {
    Expression* condition;
    BlockStmt* thenBlock;
    BlockStmt* elseBlock;

    IfStmt(Expression* t_condition, BlockStmt* t_thenBlock, BlockStmt* t_elseBlock)
        : condition(t_condition), thenBlock(t_thenBlock), elseBlock(t_elseBlock) {}
};

struct WhileStmt : Statement {
    Expression* condition;
    BlockStmt* body;

    WhileStmt(Expression* t_condition, BlockStmt* t_body)
        : condition(t_condition), body(t_body) {}
};

struct ForStmt : Statement {
    Statement* init;
    Expression* condition;
    Expression* increment;
    BlockStmt* body;

    ForStmt(Statement* t_init, Expression* t_condition, Expression* t_increment, BlockStmt* t_body)
        : init(t_init), condition(t_condition), increment(t_increment), body(t_body) {}
};