#pragma once

#include "../lexer/lexer.h"
#include "../ast/program.h"
#include "../ast/item.h"
#include "../ast/statement.h"
#include "../ast/expression.h"
#include "../token/token.h"
#include "../ast/function.h"
#include <memory>

class Parser {
public:
    Parser(Lexer& lexer);

    std::unique_ptr<Program> parseProgram();
    void printProgram(Program* program);

private:
    Lexer& lexer;

    [[noreturn]] void error(const std::string& message);

    Token consume(Kind expected, const std::string& msg);

    Item* parseItem();
    FunctionDef* parseFunction();
    ExternDecl* parseExtern();

    Statement* parseStatement();
    BlockStmt* parseBlock();

    Expression* parseExpression();
    Expression* parseLogicalOr();
    Expression* parseLogicalAnd();
    Expression* parseEquality();
    Expression* parseComparison();
    Expression* parseAdditive();
    Expression* parseMultiplicative();
    Expression* parseUnary();
    Expression* parsePostfix();
    Expression* parsePrimary();

};
