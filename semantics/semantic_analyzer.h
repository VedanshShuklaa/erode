#pragma once

#include <memory>
#include <string>
#include "scope.h"

enum class TypeKind;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    void analyzeProgram(Program* t_program);

private:
    [[noreturn]] void error(const std::string& t_message);

    void enterScope();
    void leaveScope();

    void analyzeItem(Item* t_item);
    void analyzeFunction(FunctionDef* t_func);
    void analyzeExtern(ExternDecl* t_externDecl);

    void analyzeStatement(Statement* t_stmt);
    void analyzeIf(IfStmt* t_stmt);
    void analyzeWhile(WhileStmt* t_stmt);
    void analyzeFor(ForStmt* t_stmt);
    TypeKind analyzeExpression(Expression* t_expr);

private:
    TypeKind m_currentReturnType;
    Scope* m_currentScope;
};