#pragma once

#include <memory>
#include <string>
#include "scope.h"

enum class TypeKind;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    void analyzeProgram(Program* program);

private:
    // Error handling
    [[noreturn]] void error(const std::string& message);

    // Scope management
    void enterScope();
    void leaveScope();

    // Item-level analysis
    void analyzeItem(Item* item);
    void analyzeFunction(FunctionDef* func);
    void analyzeExtern(ExternDecl* externDecl);

    // Statement & expression analysis
    void analyzeStatement(Statement* stmt);
    TypeKind analyzeExpression(Expression* expr);

private:
    TypeKind currentReturnType;
    Scope* currentScope;
};
