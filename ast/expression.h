//Expression header file for all the expression related AST nodes

#pragma once
#include "node.h"
#include "../token/token.h"
#include <string>
#include <vector>

struct Expression : Node {
    
};

struct IdentifierExpr : Expression {
    std::string name;
    IdentifierExpr(std::string t_name) : name(t_name) {}
};

struct AssignExpr : Expression {
    std::string name;
    Expression* value;
    AssignExpr(std::string t_name, Expression* t_value) : name(t_name), value(t_value) {}
};

struct IntExpr : Expression {
    int value;
    IntExpr(int t_value) : value(t_value) {}
};

struct FloatExpr : Expression {
    float value;
    FloatExpr(float t_value) : value(t_value) {}
};

struct BoolExpr : Expression {
    bool value;
    BoolExpr(bool t_value) : value(t_value) {}
};

struct CharExpr : Expression {
    char value;
    CharExpr(char t_value) : value(t_value) {}
};

struct StringExpr : Expression {
    std::string value;
    StringExpr(std::string t_value) : value(t_value) {}
};

struct UnaryExpr : Expression {
    Operator op;
    Expression* operand;

    UnaryExpr(Operator t_op, Expression* t_operand) : op(t_op), operand(t_operand) {}
};

struct BinaryExpr : Expression {
    Operator op;
    Expression* left;
    Expression* right;

    BinaryExpr(Operator t_op, Expression* t_left, Expression* t_right) : op(t_op), left(t_left), right(t_right) {}
};

struct CallExpr : Expression {
    std::string callee;
    std::vector<Expression*> arguments;

    CallExpr(std::string t_callee, std::vector<Expression*> t_arguments) : callee(t_callee), arguments(t_arguments) {}
};
