// codegen.cpp
#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>

CodeGen::CodeGen() {
    // Initialize LLVM components
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("my_module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    currentFunction = nullptr;
    
    // Start with global scope
    pushScope();
}

// Convert lexer provided typekind to LLVM types
llvm::Type* CodeGen::getLLVMType(TypeKind kind) {
    switch (kind) {
        case TypeKind::INT:
            return llvm::Type::getInt32Ty(*context);
        case TypeKind::FLOAT:
            return llvm::Type::getFloatTy(*context);
        case TypeKind::BOOL:
            return llvm::Type::getInt1Ty(*context);
        case TypeKind::CHAR:
            return llvm::Type::getInt8Ty(*context);
        case TypeKind::VOID:
            return llvm::Type::getVoidTy(*context);
        default:
            std::cerr << "Unknown type kind\n";
            return nullptr;
    }
}

// Create a stack allocation in the entry block of a function
llvm::AllocaInst* CodeGen::createEntryBlockAlloca(llvm::Function* func,
                                                   const std::string& varName,
                                                   llvm::Type* type) {
    llvm::IRBuilder<> tmpBuilder(&func->getEntryBlock(), 
                                 func->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(type, nullptr, varName);
}

// Scope management
void CodeGen::pushScope() {
    namedValues.push_back(std::map<std::string, llvm::AllocaInst*>());
}

void CodeGen::popScope() {
    if (!namedValues.empty()) {
        namedValues.pop_back();
    }
}

llvm::AllocaInst* CodeGen::findVariable(const std::string& name) {
    for (auto it = namedValues.rbegin(); it != namedValues.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return nullptr;
}

void CodeGen::dump() {
    module->print(llvm::outs(), nullptr);
}

void CodeGen::generate(Program* program) {
    generateProgram(program);
    
    std::string errStr;
    llvm::raw_string_ostream os(errStr);
    if (llvm::verifyModule(*module, &os)) {
        std::cerr << "Error: Module verification failed:\n" << os.str() << std::endl;
    }
}

void CodeGen::generateProgram(Program* program) {
    // First pass: generate all extern declarations
    for (Item* item : program->items) {
        if (auto* ext = dynamic_cast<ExternDecl*>(item)) {
            generateExtern(ext);
        }
    }
    
    // Second pass: generate all functions
    for (Item* item : program->items) {
        if (auto* func = dynamic_cast<FunctionDef*>(item)) {
            generateFunction(func);
        } else if (auto* stmt = dynamic_cast<Statement*>(item)) {
            std::cerr << "Warning: Top-level statements not supported\n";
        }
    }
}

llvm::Function* CodeGen::generateExtern(ExternDecl* ext) {
    std::vector<llvm::Type*> paramTypes;
    for (const Param& param : ext->params) {
        paramTypes.push_back(getLLVMType(param.type));
    }
    
    llvm::Type* returnType = getLLVMType(ext->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        returnType,
        paramTypes,
        false
    );
    
    llvm::Function* func = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        ext->name,
        module.get()
    );
    
    unsigned idx = 0;
    for (auto& arg : func->args()) {
        arg.setName(ext->params[idx++].name);
    }
    
    return func;
}

llvm::Function* CodeGen::generateFunction(FunctionDef* funcDef) {
    llvm::Function* func = module->getFunction(funcDef->name);
    
    if (!func) {
        std::vector<llvm::Type*> paramTypes;
        for (const Param& param : funcDef->params) {
            paramTypes.push_back(getLLVMType(param.type));
        }
        
        llvm::Type* returnType = getLLVMType(funcDef->returnType);
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            returnType,
            paramTypes,
            false
        );
        
        func = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            funcDef->name,
            module.get()
        );
        
        unsigned idx = 0;
        for (auto& arg : func->args()) {
            arg.setName(funcDef->params[idx++].name);
        }
    }
    
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(
        *context,
        "entry",
        func
    );
    builder->SetInsertPoint(entryBlock);
    
    currentFunction = func;
    
    pushScope();
    
    for (auto& arg : func->args()) {
        llvm::AllocaInst* alloca = createEntryBlockAlloca(
            func,
            arg.getName().str(),
            arg.getType()
        );
        
        builder->CreateStore(&arg, alloca);
        namedValues.back()[arg.getName().str()] = alloca;
    }
    
    generateBlock(funcDef->body, false);
    
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
    if (!currentBlock->getTerminator()) {
        if (funcDef->returnType == TypeKind::VOID) {
            builder->CreateRetVoid();
        } else {
            llvm::Type* retType = getLLVMType(funcDef->returnType);
            builder->CreateRet(llvm::Constant::getNullValue(retType));
        }
    }
    
    popScope();
    currentFunction = nullptr;
    
    std::string errStr;
    llvm::raw_string_ostream os(errStr);
    if (llvm::verifyFunction(*func, &os)) {
        std::cerr << "Error in function " << funcDef->name << ":\n" 
                  << os.str() << std::endl;
        func->print(llvm::errs());
    }
    
    return func;
}

void CodeGen::generateBlock(BlockStmt* block, bool newScope) {
    if (newScope) {
        pushScope();
    }
    
    for (Statement* stmt : block->statements) {
        generateStatement(stmt);
        
        if (builder->GetInsertBlock()->getTerminator()) {
            break;
        }
    }
    
    if (newScope) {
        popScope();
    }
}

void CodeGen::generateStatement(Statement* stmt) {
    if (auto* varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        generateVarDecl(varDecl);
    }
    else if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        generateExpression(exprStmt->expr);
    }
    else if (auto* retStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        generateReturn(retStmt);
    }
    else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        generateIf(ifStmt);
    }
    else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        generateWhile(whileStmt);
    }
    else if (auto* forStmt = dynamic_cast<ForStmt*>(stmt)) {
        generateFor(forStmt);
    }
    else if (auto* blockStmt = dynamic_cast<BlockStmt*>(stmt)) {
        generateBlock(blockStmt, true);
    }
}

void CodeGen::generateVarDecl(VarDeclStmt* stmt) {
    llvm::Type* type = getLLVMType(stmt->kind);
    llvm::AllocaInst* alloca = createEntryBlockAlloca(
        currentFunction,
        stmt->name,
        type
    );
    
    if (stmt->initializer) {
        llvm::Value* initVal = generateExpression(stmt->initializer);
        builder->CreateStore(initVal, alloca);
    }
    
    namedValues.back()[stmt->name] = alloca;
}

void CodeGen::generateReturn(ReturnStmt* stmt) {
    if (stmt->value) {
        llvm::Value* retVal = generateExpression(stmt->value);
        builder->CreateRet(retVal);
    } else {
        builder->CreateRetVoid();
    }
}

void CodeGen::generateIf(IfStmt* stmt) {
    llvm::Value* condValue = generateExpression(stmt->condition);
    
    // Convert to i1 if needed (comparison results are already i1)
    if (condValue->getType()->isIntegerTy(32)) {
        condValue = builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(*context, llvm::APInt(32, 0)),
            "ifcond"
        );
    } else if (!condValue->getType()->isIntegerTy(1)) {
        condValue = builder->CreateICmpNE(
            condValue,
            llvm::Constant::getNullValue(condValue->getType()),
            "ifcond"
        );
    }
    
    // Create blocks - initially without parent function for else and merge
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(
        *context, "then", currentFunction
    );
    llvm::BasicBlock* elseBlock = nullptr;
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "ifcont");
    
    if (stmt->elseBlock) {
        elseBlock = llvm::BasicBlock::Create(*context, "else");
        builder->CreateCondBr(condValue, thenBlock, elseBlock);
    } else {
        builder->CreateCondBr(condValue, thenBlock, mergeBlock);
    }
    
    // Generate then block
    builder->SetInsertPoint(thenBlock);
    generateBlock(stmt->thenBlock, true);
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBlock);
    }
    
    // Generate else block if it exists
    if (stmt->elseBlock) {
        // Insert else block into function (LLVM 16+ compatible)
        elseBlock->insertInto(currentFunction);
        builder->SetInsertPoint(elseBlock);
        generateBlock(stmt->elseBlock, true);
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(mergeBlock);
        }
    }
    
    // Insert merge block into function
    mergeBlock->insertInto(currentFunction);
    builder->SetInsertPoint(mergeBlock);
}

void CodeGen::generateWhile(WhileStmt* stmt) {
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(
        *context, "whilecond", currentFunction
    );
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(*context, "whilebody");
    llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(*context, "afterwhile");
    
    builder->CreateBr(condBlock);
    
    // Generate condition block
    builder->SetInsertPoint(condBlock);
    llvm::Value* condValue = generateExpression(stmt->condition);
    
    // Convert to i1 if needed
    if (condValue->getType()->isIntegerTy(32)) {
        condValue = builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(*context, llvm::APInt(32, 0)),
            "whilecond"
        );
    } else if (!condValue->getType()->isIntegerTy(1)) {
        condValue = builder->CreateICmpNE(
            condValue,
            llvm::Constant::getNullValue(condValue->getType()),
            "whilecond"
        );
    }
    builder->CreateCondBr(condValue, bodyBlock, afterBlock);
    
    // Generate body block
    bodyBlock->insertInto(currentFunction);
    builder->SetInsertPoint(bodyBlock);
    generateBlock(stmt->body, true);
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(condBlock);
    }
    
    // Continue with after block
    afterBlock->insertInto(currentFunction);
    builder->SetInsertPoint(afterBlock);
}

void CodeGen::generateFor(ForStmt* stmt) {
    pushScope();
    
    // Generate initialization
    if (stmt->init) {
        generateStatement(stmt->init);
    }
    
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(
        *context, "forcond", currentFunction
    );
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(*context, "forbody");
    llvm::BasicBlock* incBlock = llvm::BasicBlock::Create(*context, "forinc");
    llvm::BasicBlock* afterBlock = llvm::BasicBlock::Create(*context, "afterfor");
    
    builder->CreateBr(condBlock);
    
    // Generate condition block
    builder->SetInsertPoint(condBlock);
    if (stmt->condition) {
        llvm::Value* condValue = generateExpression(stmt->condition);
        
        // Convert to i1 if needed
        if (condValue->getType()->isIntegerTy(32)) {
            condValue = builder->CreateICmpNE(
                condValue,
                llvm::ConstantInt::get(*context, llvm::APInt(32, 0)),
                "forcond"
            );
        } else if (!condValue->getType()->isIntegerTy(1)) {
            condValue = builder->CreateICmpNE(
                condValue,
                llvm::Constant::getNullValue(condValue->getType()),
                "forcond"
            );
        }
        builder->CreateCondBr(condValue, bodyBlock, afterBlock);
    } else {
        builder->CreateBr(bodyBlock);
    }
    
    // Generate body block
    bodyBlock->insertInto(currentFunction);
    builder->SetInsertPoint(bodyBlock);
    generateBlock(stmt->body, false);
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(incBlock);
    }
    
    // Generate increment block
    incBlock->insertInto(currentFunction);
    builder->SetInsertPoint(incBlock);
    if (stmt->increment) {
        generateExpression(stmt->increment);
    }
    builder->CreateBr(condBlock);
    
    // Continue with after block
    afterBlock->insertInto(currentFunction);
    builder->SetInsertPoint(afterBlock);
    
    popScope();
}

llvm::Value* CodeGen::generateExpression(Expression* expr) {
    if (auto* intExpr = dynamic_cast<IntExpr*>(expr)) {
        return llvm::ConstantInt::get(
            *context,
            llvm::APInt(32, intExpr->value, true)
        );
    }
    else if (auto* floatExpr = dynamic_cast<FloatExpr*>(expr)) {
        return llvm::ConstantFP::get(*context, llvm::APFloat(floatExpr->value));
    }
    else if (auto* boolExpr = dynamic_cast<BoolExpr*>(expr)) {
        return llvm::ConstantInt::get(
            *context,
            llvm::APInt(1, boolExpr->value ? 1 : 0)
        );
    }
    else if (auto* charExpr = dynamic_cast<CharExpr*>(expr)) {
        return llvm::ConstantInt::get(
            *context,
            llvm::APInt(8, charExpr->value)
        );
    }
    else if (auto* stringExpr = dynamic_cast<StringExpr*>(expr)) {
        return builder->CreateGlobalStringPtr(stringExpr->value);
    }
    else if (auto* identExpr = dynamic_cast<IdentifierExpr*>(expr)) {
        return generateIdentifier(identExpr);
    }
    else if (auto* binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
        return generateBinaryExpr(binaryExpr);
    }
    else if (auto* unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
        return generateUnaryExpr(unaryExpr);
    }
    else if (auto* callExpr = dynamic_cast<CallExpr*>(expr)) {
        return generateCallExpr(callExpr);
    }
    else if (auto* assignExpr = dynamic_cast<AssignExpr*>(expr)) {
        return generateAssignExpr(assignExpr);
    }
    
    std::cerr << "Unknown expression type\n";
    return nullptr;
}

llvm::Value* CodeGen::generateIdentifier(IdentifierExpr* expr) {
    llvm::AllocaInst* alloca = findVariable(expr->name);
    if (!alloca) {
        std::cerr << "Unknown variable: " << expr->name << std::endl;
        return nullptr;
    }
    
    return builder->CreateLoad(alloca->getAllocatedType(), alloca, expr->name);
}

llvm::Value* CodeGen::generateAssignExpr(AssignExpr* expr) {
    llvm::Value* val = generateExpression(expr->value);
    llvm::AllocaInst* alloca = findVariable(expr->name);
    
    if (!alloca) {
        std::cerr << "Unknown variable: " << expr->name << std::endl;
        return nullptr;
    }
    
    builder->CreateStore(val, alloca);
    return val;
}

llvm::Value* CodeGen::generateBinaryExpr(BinaryExpr* expr) {
    llvm::Value* left = generateExpression(expr->left);
    llvm::Value* right = generateExpression(expr->right);
    
    if (!left || !right) {
        return nullptr;
    }
    
    bool isFloat = left->getType()->isFloatingPointTy() || 
                   right->getType()->isFloatingPointTy();
    
    switch (expr->op) {
        case Operator::Plus:
            return isFloat ? builder->CreateFAdd(left, right, "addtmp")
                          : builder->CreateAdd(left, right, "addtmp");
        
        case Operator::Minus:
            return isFloat ? builder->CreateFSub(left, right, "subtmp")
                          : builder->CreateSub(left, right, "subtmp");
        
        case Operator::Multiply:
            return isFloat ? builder->CreateFMul(left, right, "multmp")
                          : builder->CreateMul(left, right, "multmp");
        
        case Operator::Divide:
            return isFloat ? builder->CreateFDiv(left, right, "divtmp")
                          : builder->CreateSDiv(left, right, "divtmp");
        
        case Operator::Less:
            return isFloat ? builder->CreateFCmpULT(left, right, "cmptmp")
                          : builder->CreateICmpSLT(left, right, "cmptmp");
        
        case Operator::Greater:
            return isFloat ? builder->CreateFCmpUGT(left, right, "cmptmp")
                          : builder->CreateICmpSGT(left, right, "cmptmp");
        
        case Operator::LessEqual:
            return isFloat ? builder->CreateFCmpULE(left, right, "cmptmp")
                          : builder->CreateICmpSLE(left, right, "cmptmp");
        
        case Operator::GreaterEqual:
            return isFloat ? builder->CreateFCmpUGE(left, right, "cmptmp")
                          : builder->CreateICmpSGE(left, right, "cmptmp");
        
        case Operator::EqualEqual:
            return isFloat ? builder->CreateFCmpUEQ(left, right, "cmptmp")
                          : builder->CreateICmpEQ(left, right, "cmptmp");
        
        case Operator::NotEqual:
            return isFloat ? builder->CreateFCmpUNE(left, right, "cmptmp")
                          : builder->CreateICmpNE(left, right, "cmptmp");
        
        case Operator::AndAnd:
            return builder->CreateAnd(left, right, "andtmp");
        
        case Operator::OrOr:
            return builder->CreateOr(left, right, "ortmp");
        
        default:
            std::cerr << "Unknown binary operator\n";
            return nullptr;
    }
}

llvm::Value* CodeGen::generateUnaryExpr(UnaryExpr* expr) {
    llvm::Value* operand = generateExpression(expr->operand);
    
    if (!operand) {
        return nullptr;
    }
    
    switch (expr->op) {
        case Operator::Minus:
            if (operand->getType()->isFloatingPointTy()) {
                return builder->CreateFNeg(operand, "negtmp");
            } else {
                return builder->CreateNeg(operand, "negtmp");
            }
        
        case Operator::Not:
            return builder->CreateNot(operand, "nottmp");
        
        default:
            std::cerr << "Unknown unary operator\n";
            return nullptr;
    }
}

llvm::Value* CodeGen::generateCallExpr(CallExpr* expr) {
    llvm::Function* calleeFunc = module->getFunction(expr->callee);
    
    if (!calleeFunc) {
        std::cerr << "Unknown function: " << expr->callee << std::endl;
        return nullptr;
    }
    
    if (calleeFunc->arg_size() != expr->arguments.size()) {
        std::cerr << "Incorrect number of arguments for " << expr->callee << std::endl;
        return nullptr;
    }
    
    std::vector<llvm::Value*> args;
    for (Expression* argExpr : expr->arguments) {
        llvm::Value* argVal = generateExpression(argExpr);
        if (!argVal) {
            return nullptr;
        }
        args.push_back(argVal);
    }
    
    if (calleeFunc->getReturnType()->isVoidTy()) {
        return builder->CreateCall(calleeFunc, args);
    }
    return builder->CreateCall(calleeFunc, args, "calltmp");
}