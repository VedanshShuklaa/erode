#include "lexer.h";
#include <cstdint>
#include <string>
#include <iostream>

struct Node {
    virtual ~Node() = default;
};

struct Program : Node {
    std::vector<Node*> items;
};

struct Statement : Node {};

struct BlockStmt : Statement {
    std::vector<Statement*> statements;
};

enum class TypeKind {
    INT,
    FLOAT,
    STRING,
    BOOL,
    CHAR
};

struct VarDeclStmt : Statement {
    TypeKind kind;
    std::string name;
    Node* initializer;
};

struct ExprStmt : Statement {
    Node* expr;
};

struct Expression : Node {};

enum class BinaryOp {
    Plus,
    Minus,
    PlusPlus,
    MinusMinus,
    Multiply,
    Divide,
    EqualEqual,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    PlusEqual,
    MinusEqual,
    AndAnd,
    OrOr,
    And,
    Or,
    Not,
    Equal,
};

struct BinaryExpr : Expression {
    BinaryOp op;
    Expression* left;
    Expression* right;
};

enum class UnaryOp {
    Negate,
    Not
};

struct UnaryExpr : Expression {
    UnaryOp op;
    Expression* operand;
};
