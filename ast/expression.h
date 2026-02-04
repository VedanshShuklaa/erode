#pragma once
#include "node.h"
#include "../token/token.h"
#include <string>
#include <vector>

struct Expression : Node {};

struct IdentifierExpr : Expression {
    std::string name;
    IdentifierExpr(std::string name) : name(name) {}
};

struct IntExpr : Expression {
    int value;
    IntExpr(int value) : value(value) {}
};

struct FloatExpr : Expression {
    float value;
    FloatExpr(float value) : value(value) {}
};

struct BoolExpr : Expression {
    bool value;
    BoolExpr(bool value) : value(value) {}
};

struct CharExpr : Expression {
    char value;
    CharExpr(char value) : value(value) {}
};

struct StringExpr : Expression {
    std::string value;
    StringExpr(std::string value) : value(value) {}
};

struct UnaryExpr : Expression {
    Operator op;
    Expression* operand;

    UnaryExpr(Operator op, Expression* operand) : op(op), operand(operand) {}
};

struct BinaryExpr : Expression {
    Operator op;
    Expression* left;
    Expression* right;

    BinaryExpr(Operator op, Expression* left, Expression* right) : op(op), left(left), right(right) {}
};

struct CallExpr : Expression {
    std::string callee;
    std::vector<Expression*> arguments;

    CallExpr(std::string callee, std::vector<Expression*> arguments) : callee(callee), arguments(arguments) {}
};
