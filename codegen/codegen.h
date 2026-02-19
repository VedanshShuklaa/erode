#pragma once
#include "../ast/program.h"
#include "../ast/function.h"
#include "../ast/statement.h"
#include "../ast/expression.h"
#include <algorithm>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <map>
#include <string>
#include <memory>

class CodeGen {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    std::vector<std::map<std::string, llvm::AllocaInst*>> namedValues;

    llvm::Function* currentFunction; 

    llvm::Type* getLLVMType(TypeKind type);

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* func, 
                                             const std::string& varName, 
                                             llvm::Type* type);

    // Code generation methods for each AST node type
    void generateProgram(Program* program);
    llvm::Function* generateFunction(FunctionDef* func);
    llvm::Function* generateExtern(ExternDecl* ext);
    void generateStatement(Statement* stmt);
    void generateBlock(BlockStmt* block, bool newScope = true);
    llvm::Value* generateExpression(Expression* expr);
    
    // Specific statement generators
    void generateVarDecl(VarDeclStmt* stmt);
    void generateReturn(ReturnStmt* stmt);
    void generateIf(IfStmt* stmt);
    void generateWhile(WhileStmt* stmt);
    void generateFor(ForStmt* stmt);
    
    // Specific expression generators
    llvm::Value* generateBinaryExpr(BinaryExpr* expr);
    llvm::Value* generateUnaryExpr(UnaryExpr* expr);
    llvm::Value* generateCallExpr(CallExpr* expr);
    llvm::Value* generateAssignExpr(AssignExpr* expr);
    llvm::Value* generateIdentifier(IdentifierExpr* expr);

    // Scope management
    void pushScope();
    void popScope();
    llvm::AllocaInst* findVariable(const std::string& name);

public:
    CodeGen();
    void generate(Program* program);
    void dump();  // Print the generated IR
    llvm::Module* getModule() { return module.get(); }
};