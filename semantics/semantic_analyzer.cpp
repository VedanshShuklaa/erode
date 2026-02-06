#include "scope.h"
#include <iostream>
#include <cstdlib>
#include "semantic_analyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {
    currentScope = new Scope();
}

void SemanticAnalyzer::analyzeProgram(Program* program)
{
    // First pass: declare all functions to avoid forward references
    for (auto& item : program->items) 
    {
        if (auto func = dynamic_cast<FunctionDef*>(item)) {
            Symbol sym;
            sym.name = func->name;
            sym.type = TypeKind::INT;
            sym.isFunction = true;
            for (auto& p : func->params)
                sym.params.push_back(p.type);
            if (!currentScope->insert(sym.name, sym)) {
                error("Redefinition of function " + func->name);
            }
        }
    }
    for (auto& item : program->items) {
        analyzeItem(item);
    }
}

void SemanticAnalyzer::error(const std::string& message) {
    std::cerr << "Semantic Error: " << message << std::endl;
    exit(1);
}

void SemanticAnalyzer::enterScope() {
    currentScope = new Scope(currentScope);
}

void SemanticAnalyzer::leaveScope() {
    if( !currentScope )
        error("Scope not found");
    if( !currentScope->parent )
        error("Parent scope not found");
    currentScope = currentScope->parent;
}

void SemanticAnalyzer::analyzeItem(Item* item)
{
    if (auto externDecl = dynamic_cast<ExternDecl*>(item)) {
        analyzeExtern(externDecl);
    }
    else if (auto stmt = dynamic_cast<Statement*>(item)) {
        analyzeStatement(stmt);
    }
    else if (auto func = dynamic_cast<FunctionDef*>(item)) {
        analyzeFunction(func);
    }
}

void SemanticAnalyzer::analyzeFunction(FunctionDef* func)
{
    enterScope();
    currentReturnType = func->returnType;
    for (auto& param : func->params) {
        Symbol sym;
        sym.name = param.name;
        sym.type = param.type;
        sym.isFunction = false;
        if( !currentScope->insert(sym.name, sym) ) {
            error("Redefinition of parameter " + param.name);
        }
    }
    for (auto& stmt : func->body->statements) {
        analyzeStatement(stmt);
    }
    leaveScope();
}

void SemanticAnalyzer::analyzeExtern(ExternDecl* externDecl)
{
    if (currentScope->parent != nullptr)
        error("External declarations must be at the top level");
    
    Symbol sym;
    sym.name = externDecl->name;
    sym.isFunction = true;
    for (auto& param : externDecl->params)
        sym.params.push_back(param.type);
    sym.type = externDecl->returnType;
    if( !currentScope->insert(sym.name, sym) ) {
        error("Redefinition of external declaration " + externDecl->name);
    }
}

void SemanticAnalyzer::analyzeStatement(Statement* stmt)
{
    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        analyzeExpression(exprStmt->expr);
    }
    else if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        Symbol sym;
        sym.name = varDecl->name;
        sym.type = varDecl->kind;
        sym.isFunction = false;
        if( !currentScope->insert(sym.name, sym) ) {
            error("Redefinition of variable " + varDecl->name);
        }
        if(varDecl->initializer)
        {
            TypeKind initType = analyzeExpression(varDecl->initializer);
            if(initType != varDecl->kind)
                error("Type mismatch in variable declaration");
        }
    }
    else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        if(returnStmt->value)
        {
            TypeKind returnType = analyzeExpression(returnStmt->value);
            if(returnType != currentReturnType)
                error("Type mismatch in return statement");
        } else {
            if(currentReturnType != TypeKind::VOID)
                error("Return statement expected");
        }
    } 
    else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        enterScope();
        for (auto& stmt : blockStmt->statements) {
            analyzeStatement(stmt);
        }
        leaveScope();
    }
}

TypeKind SemanticAnalyzer::analyzeExpression(Expression* expr)
{
    if (auto* e = dynamic_cast<IntExpr*>(expr)) return TypeKind::INT;
    if (auto* e = dynamic_cast<FloatExpr*>(expr)) return TypeKind::FLOAT;
    if (auto* e = dynamic_cast<BoolExpr*>(expr)) return TypeKind::BOOL;
    if (auto* e = dynamic_cast<StringExpr*>(expr)) return TypeKind::STRING;
    if (auto* e = dynamic_cast<CharExpr*>(expr)) return TypeKind::CHAR;
    if (auto e = dynamic_cast<IdentifierExpr*>(expr)) {
        if (currentScope->lookup(e->name)) {
            return currentScope->lookup(e->name)->type;
        }
        error("Undefined variable " + e->name);
    }
    else if (auto e = dynamic_cast<BinaryExpr*>(expr)) {
        TypeKind leftType = analyzeExpression(e->left);
        TypeKind rightType = analyzeExpression(e->right);
        if(leftType != rightType)
            error("Type mismatch in binary expression");
        return leftType;
    } else if (auto e = dynamic_cast<UnaryExpr*>(expr)) {
        TypeKind operandType = analyzeExpression(e->operand);
        if(e->op == Operator::PlusPlus || e->op == Operator::MinusMinus)
        {
            if(operandType != TypeKind::INT)
                error("Operand of increment/decrement must be an integer");
        }
    } 
    else if (auto e = dynamic_cast<CallExpr*>(expr)) 
    {
        if (auto func = currentScope->lookup(e->callee)) {
            if(func->isFunction)
            {
                if(func->params.size() != e->arguments.size())
                {
                    error("Argument count mismatch in function call");
                }
            }
            else
            {
                error("Call to non-function " + e->callee);
            }
            for(auto& arg : e->arguments)
            {
                analyzeExpression(arg);
            }
            return func->type;
        }
        error("Undefined function " + e->callee);
    }
    error("Invalid expression");
}
