#include "scope.h"
#include <iostream>
#include <cstdlib>
#include "semantic_analyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {
    m_currentScope = new Scope();
}

void SemanticAnalyzer::analyzeProgram(Program* program)
{
    // First pass: declare all functions to avoid forward references
    for (auto& item : program->items) 
    {
        if (auto func = dynamic_cast<FunctionDef*>(item)) {
            Symbol sym;
            sym.name = func->name;
            sym.type = func->returnType;
            sym.isFunction = true;
            for (auto& p : func->params)
                sym.params.push_back(p.type);
            if (!m_currentScope->insert(sym.name, sym)) {
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
    m_currentScope = new Scope(m_currentScope);
}

void SemanticAnalyzer::leaveScope() {
    if( !m_currentScope )
        error("Scope not found");
    if( !m_currentScope->parent )
        error("Parent scope not found");
    Scope* old = m_currentScope;
    m_currentScope = m_currentScope->parent;
    delete old;
}

void SemanticAnalyzer::analyzeItem(Item* item)
{
    if (auto externDecl = dynamic_cast<ExternDecl*>(item)) {
        analyzeExtern(externDecl);
    }
    else if (auto func = dynamic_cast<FunctionDef*>(item)) {
        analyzeFunction(func);
    }
    else if (auto stmt = dynamic_cast<Statement*>(item)) {
        analyzeStatement(stmt);
    }
}

void SemanticAnalyzer::analyzeFunction(FunctionDef* func)
{
    enterScope();
    m_currentReturnType = func->returnType;
    for (auto& param : func->params) {
        Symbol sym;
        sym.name = param.name;
        sym.type = param.type;
        sym.isFunction = false;
        if( !m_currentScope->insert(sym.name, sym) ) {
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
    if (m_currentScope->parent != nullptr)
        error("External declarations must be at the top level");
    
    Symbol sym;
    sym.name = externDecl->name;
    sym.isFunction = true;
    for (auto& param : externDecl->params)
        sym.params.push_back(param.type);
    sym.type = externDecl->returnType;
    if( !m_currentScope->insert(sym.name, sym) ) {
        error("Redefinition of external declaration " + externDecl->name);
    }
}

void SemanticAnalyzer::analyzeIf(IfStmt* stmt)
{
    TypeKind condType = analyzeExpression(stmt->condition);
    if (condType != TypeKind::BOOL)
        error("Condition of if statement must be a boolean");
    
    enterScope();
    for (auto& s : stmt->thenBlock->statements) {
        analyzeStatement(s);
    }
    leaveScope();
    
    if (stmt->elseBlock) {
        enterScope();
        for (auto& s : stmt->elseBlock->statements) {
            analyzeStatement(s);
        }
        leaveScope();
    }
}

void SemanticAnalyzer::analyzeWhile(WhileStmt* stmt)
{
    TypeKind condType = analyzeExpression(stmt->condition);
    if (condType != TypeKind::BOOL)
        error("Condition of while statement must be a boolean");
    
    enterScope();
    for (auto& s : stmt->body->statements) {
        analyzeStatement(s);
    }
    leaveScope();
}

void SemanticAnalyzer::analyzeFor(ForStmt* stmt)
{
    enterScope();
    
    if (stmt->init) {
        analyzeStatement(stmt->init);
    }
    
    if (stmt->condition) {
        TypeKind condType = analyzeExpression(stmt->condition);
        if (condType != TypeKind::BOOL)
            error("Condition of for statement must be a boolean");
    }
    
    if (stmt->increment) {
        analyzeExpression(stmt->increment);
    }
    
    for (auto& s : stmt->body->statements) {
        analyzeStatement(s);
    }
    
    leaveScope();
}

void SemanticAnalyzer::analyzeStatement(Statement* stmt)
{
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        analyzeIf(ifStmt);
    }
    else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        analyzeWhile(whileStmt);
    }
    else if (auto forStmt = dynamic_cast<ForStmt*>(stmt)) {
        analyzeFor(forStmt);
    }
    else if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        analyzeExpression(exprStmt->expr);
    }
    else if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        Symbol sym;
        sym.name = varDecl->name;
        sym.type = varDecl->kind;
        sym.isFunction = false;
        if( !m_currentScope->insert(sym.name, sym) ) {
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
            if(returnType != m_currentReturnType)
                error("Type mismatch in return statement");
        } else {
            if(m_currentReturnType != TypeKind::VOID)
                error("Return statement expected");
        }
    } 
    else if (auto blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        enterScope();
        for (auto& s : blockStmt->statements) {
            analyzeStatement(s);
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
        if (auto sym = m_currentScope->lookup(e->name)) {
            return sym->type;
        }
        error("Undefined variable " + e->name);
    }
    
    if (auto e = dynamic_cast<BinaryExpr*>(expr)) {
        TypeKind leftType = analyzeExpression(e->left);
        TypeKind rightType = analyzeExpression(e->right);
        
        // Comparison and logical operators return bool
        if (e->op == Operator::EqualEqual || e->op == Operator::NotEqual ||
            e->op == Operator::Less || e->op == Operator::Greater ||
            e->op == Operator::LessEqual || e->op == Operator::GreaterEqual) {
            if (leftType != rightType)
                error("Type mismatch in comparison expression");
            return TypeKind::BOOL;
        }
        
        if (e->op == Operator::AndAnd || e->op == Operator::OrOr) {
            if (leftType != TypeKind::BOOL || rightType != TypeKind::BOOL)
                error("Logical operators require boolean operands");
            return TypeKind::BOOL;
        }
        
        // Arithmetic operators
        if (leftType != rightType)
            error("Type mismatch in binary expression");
        return leftType;
    }
    
    if (auto e = dynamic_cast<UnaryExpr*>(expr)) {
        TypeKind operandType = analyzeExpression(e->operand);
        if (e->op == Operator::Not) {
            if (operandType != TypeKind::BOOL)
                error("Operand of '!' must be a boolean");
            return TypeKind::BOOL;
        }
        if (e->op == Operator::Minus) {
            if (operandType != TypeKind::INT && operandType != TypeKind::FLOAT)
                error("Operand of unary '-' must be numeric");
            return operandType;
        }
        if (e->op == Operator::PlusPlus || e->op == Operator::MinusMinus) {
            if (operandType != TypeKind::INT)
                error("Operand of increment/decrement must be an integer");
            return TypeKind::INT;
        }
        return operandType;
    }
    
    if (auto e = dynamic_cast<CallExpr*>(expr)) {
        if (auto func = m_currentScope->lookup(e->callee)) {
            if (func->isFunction) {
                if (func->params.size() != e->arguments.size()) {
                    error("Argument count mismatch in function call");
                }
                for (size_t i = 0; i < e->arguments.size(); i++) {
                    TypeKind argType = analyzeExpression(e->arguments[i]);
                    if (argType != func->params[i]) {
                        error("Argument type mismatch in function call");
                    }
                }
            } else {
                error("Call to non-function " + e->callee);
            }
            return func->type;
        }
        error("Undefined function " + e->callee);
    }
    
    if (auto e = dynamic_cast<AssignExpr*>(expr)) {
        if (auto sym = m_currentScope->lookup(e->name)) {
            if (sym->isFunction) {
                error("Cannot assign to function " + e->name);
            }
            TypeKind valueType = analyzeExpression(e->value);
            if (valueType != sym->type) {
                error("Type mismatch in assignment");
            }
            return sym->type;
        }
        error("Undefined variable " + e->name);
    }

    error("Invalid expression");
}