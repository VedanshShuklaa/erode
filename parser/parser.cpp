#include "../lexer/lexer.h"
#include "../ast/program.h"
#include "../ast/item.h"
#include "../ast/statement.h"
#include "../ast/expression.h"
#include "../token/token.h"
#include "../ast/function.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include "parser.h"
#include "parser_helper.h"


[[noreturn]] void Parser::error(const std::string& message) {
    std::cerr << "Parse error: " << message << std::endl;
    exit(1);
}

Parser::Parser(Lexer& lexer) : lexer(lexer) {}

Token Parser::consume(Kind expected, const std::string& msg) {
    Token tok = lexer.current();
    if (tok.kind != expected) {
        error(msg);
    }
    lexer.next();
    return tok;
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::unique_ptr<Program>(new Program());
    while (lexer.current().kind != Kind::tok_eof) {
        program->items.push_back(parseItem());
    }
    return program;
}

Item* Parser::parseItem() {
    if (lexer.current().kind == Kind::tok_def) {
        consume(Kind::tok_def, "Expected 'def' keyword");
        return parseFunction();
    }
    if (lexer.current().kind == Kind::tok_extern) {
        consume(Kind::tok_extern, "Expected 'extern' keyword");
        return parseExtern();
    }
    return parseStatement();
}

FunctionDef* Parser::parseFunction() {
    Token token = consume(Kind::tok_identifier, "Expected identifier after def");
    std::string name = std::get<std::string>(token.value);
    consume(Kind::tok_lparen, "Expected '(' after function name");
    token = lexer.current();
    std::vector<Param> params;
    while (token.kind != Kind::tok_rparen) {
        if (isType(token.kind)) {
            TypeKind type = getTypeKind(token.kind);
            lexer.next();
            token = lexer.current();
            
            if (token.kind != Kind::tok_identifier) {
                error("Expected identifier after type in function parameter");
            }
            std::string paramName = std::get<std::string>(token.value);
            params.push_back({type, paramName});
            
            lexer.next();
            token = lexer.current();
            
            if (token.kind == Kind::tok_comma) {
                lexer.next();
                token = lexer.current();
            } else if (token.kind != Kind::tok_rparen) {
                error("Expected ',' or ')' after parameter");
            }
        } else {
            error("Expected type in function parameter");
        }
    }
    consume(Kind::tok_rparen, "Expected ')' after function parameters");

    TypeKind returnType = TypeKind::VOID;

    if (lexer.current().kind == Kind::tok_arrow) {
        lexer.next();
        token = lexer.current();
        if (!isType(token.kind)) {
            error("Expected type after '->' in function return type");
        }
        returnType = getTypeKind(token.kind);
        lexer.next();
        token = lexer.current();
    }

    consume(Kind::tok_lbrace, "Expected '{' after function parameters");
    
    BlockStmt* body = parseBlock();
    
    consume(Kind::tok_rbrace, "Expected '}' to close function body");
    
    return new FunctionDef(name, params, body, returnType);
}

BlockStmt* Parser::parseBlock() {
    BlockStmt* block = new BlockStmt();
    
    while (lexer.current().kind != Kind::tok_rbrace) {
        block->statements.push_back(parseStatement());
    }
    
    return block;
}

ExternDecl* Parser::parseExtern() {
    TypeKind returnType = TypeKind::VOID;
    if (isType(lexer.current().kind)) {
        returnType = getTypeKind(lexer.current().kind);
        lexer.next();
    }
    Token token = consume(Kind::tok_identifier, "Expected identifier after extern");
    std::string name = std::get<std::string>(token.value);
    
    consume(Kind::tok_lparen, "Expected '(' after extern identifier");
    token = lexer.current();
    
    std::vector<Param> params = {};
    
    while (token.kind != Kind::tok_rparen) {
        if (isType(token.kind)) {
            TypeKind type = getTypeKind(token.kind);
            lexer.next();
            token = lexer.current();
            
            if (token.kind != Kind::tok_identifier) {
                error("Expected identifier after type in extern parameter");
            }
            std::string paramName = std::get<std::string>(token.value);
            params.push_back({type, paramName});
            
            lexer.next();
            token = lexer.current();
            
            if (token.kind == Kind::tok_comma) {
                lexer.next();
                token = lexer.current();
            } else if (token.kind != Kind::tok_rparen) {
                error("Expected ',' or ')' after parameter");
            }
        } else {
            error("Expected type in extern parameter");
        }
    }
    consume(Kind::tok_rparen, "Expected ')' after extern parameters");
    consume(Kind::tok_semicolon, "Expected ';' after extern declaration");
            
    ExternDecl* externDecl = new ExternDecl(name, params, returnType);
    return externDecl;
}

IfStmt* Parser::parseIf() {
    consume(Kind::tok_lparen, "Expected '(' after 'if'");
    Expression* condition = parseExpression();
    consume(Kind::tok_rparen, "Expected ')' after if condition");
    
    consume(Kind::tok_lbrace, "Expected '{' after if condition");
    BlockStmt* thenBlock = parseBlock();
    consume(Kind::tok_rbrace, "Expected '}' after if body");
    
    BlockStmt* elseBlock = nullptr;
    if (lexer.current().kind == Kind::tok_else) {
        lexer.next();
        consume(Kind::tok_lbrace, "Expected '{' after 'else'");
        elseBlock = parseBlock();
        consume(Kind::tok_rbrace, "Expected '}' after else body");
    }
    
    return new IfStmt(condition, thenBlock, elseBlock);
}

WhileStmt* Parser::parseWhile() {
    consume(Kind::tok_lparen, "Expected '(' after 'while'");
    Expression* condition = parseExpression();
    consume(Kind::tok_rparen, "Expected ')' after while condition");
    
    consume(Kind::tok_lbrace, "Expected '{' after while condition");
    BlockStmt* body = parseBlock();
    consume(Kind::tok_rbrace, "Expected '}' after while body");
    
    return new WhileStmt(condition, body);
}

ForStmt* Parser::parseFor() {
    consume(Kind::tok_lparen, "Expected '(' after 'for'");
    
    // Parse initializer
    Statement* init = nullptr;
    if (lexer.current().kind != Kind::tok_semicolon) {
        if (isType(lexer.current().kind)) {
            TypeKind type = getTypeKind(lexer.current().kind);
            lexer.next();
            Token nameTok = consume(Kind::tok_identifier, "Expected identifier in for init");
            Expression* initExpr = nullptr;
            if (lexer.current().kind == Kind::tok_operator &&
                std::get<Operator>(lexer.current().value) == Operator::Equal) {
                lexer.next();
                initExpr = parseExpression();
            }
            init = new VarDeclStmt(type, std::get<std::string>(nameTok.value), initExpr);
        } else {
            Expression* expr = parseExpression();
            init = new ExprStmt(expr);
        }
    }
    consume(Kind::tok_semicolon, "Expected ';' after for initializer");
    
    // Parse condition
    Expression* condition = nullptr;
    if (lexer.current().kind != Kind::tok_semicolon) {
        condition = parseExpression();
    }
    consume(Kind::tok_semicolon, "Expected ';' after for condition");
    
    // Parse increment
    Expression* increment = nullptr;
    if (lexer.current().kind != Kind::tok_rparen) {
        increment = parseExpression();
    }
    consume(Kind::tok_rparen, "Expected ')' after for clauses");
    
    consume(Kind::tok_lbrace, "Expected '{' after for header");
    BlockStmt* body = parseBlock();
    consume(Kind::tok_rbrace, "Expected '}' after for body");
    
    return new ForStmt(init, condition, increment, body);
}

Statement* Parser::parseStatement() {
    if (lexer.current().kind == Kind::tok_if) {
        consume(Kind::tok_if, "Expected 'if'");
        return parseIf();
    }

    if (lexer.current().kind == Kind::tok_while) {
        consume(Kind::tok_while, "Expected 'while'");
        return parseWhile();
    }

    if (lexer.current().kind == Kind::tok_for) {
        consume(Kind::tok_for, "Expected 'for'");
        return parseFor();
    }

    if (lexer.current().kind == Kind::tok_return) {
        consume(Kind::tok_return, "Expected 'return'");
        Expression* value = nullptr;
        if (lexer.current().kind != Kind::tok_semicolon) {
            value = parseExpression();
        }
        consume(Kind::tok_semicolon, "Expected ';'");
        return new ReturnStmt(value);
    }

    if (isType(lexer.current().kind)) {
        TypeKind type = getTypeKind(lexer.current().kind);
        lexer.next();
        Token nameTok = consume(Kind::tok_identifier, "Expected identifier");
        Expression* init = nullptr;
        if (lexer.current().kind == Kind::tok_operator &&
            std::get<Operator>(lexer.current().value) == Operator::Equal) {
            lexer.next();
            init = parseExpression();
        }
        consume(Kind::tok_semicolon, "Expected ';'");
        auto* stmt = new VarDeclStmt(type, std::get<std::string>(nameTok.value), init);
        return stmt;
    }

    Expression* expr = parseExpression();
    consume(Kind::tok_semicolon, "Expected ';'");
    
    return new ExprStmt(expr);
}

Expression* Parser::parseExpression() {
    return parseAssignment();
}

Expression* Parser::parseAssignment() {
    Expression* left = parseLogicalOr();
    
    if (lexer.current().kind == Kind::tok_operator &&
        std::get<Operator>(lexer.current().value) == Operator::Equal) {
        lexer.next();
        Expression* right = parseAssignment(); // right-associative
        
        if (auto* ident = dynamic_cast<IdentifierExpr*>(left)) {
            return new AssignExpr(ident->name, right);
        } else {
            error("Left side of assignment must be a variable");
        }
    }
    
    return left;
}

Expression* Parser::parseLogicalOr() {
    Expression* left = parseLogicalAnd();
    while (lexer.current().kind == Kind::tok_operator &&
        std::get<Operator>(lexer.current().value) == Operator::OrOr) {
        lexer.next();
        Expression* right = parseLogicalAnd();
        left = new BinaryExpr(Operator::OrOr, left, right);
    }
    return left;
}

Expression* Parser::parseLogicalAnd() {
    Expression* left = parseEquality();
    while (lexer.current().kind == Kind::tok_operator &&
        std::get<Operator>(lexer.current().value) == Operator::AndAnd) {
        lexer.next();
        Expression* right = parseEquality();
        left = new BinaryExpr(Operator::AndAnd, left, right);
    }
    return left;
}

Expression* Parser::parseEquality() {
    Expression* left = parseComparison();
    while (lexer.current().kind == Kind::tok_operator) {
        Operator op = std::get<Operator>(lexer.current().value);
        if (op != Operator::EqualEqual && op != Operator::NotEqual)
            break;
        lexer.next();
        Expression* right = parseComparison();
        left = new BinaryExpr(op, left, right);
    }
    return left;
}

Expression* Parser::parseComparison() {
    Expression* left = parseAdditive();
    while (lexer.current().kind == Kind::tok_operator) {
        Operator op = std::get<Operator>(lexer.current().value);
        if (op != Operator::Less && op != Operator::Greater && 
            op != Operator::LessEqual && op != Operator::GreaterEqual)
            break;
        lexer.next();
        Expression* right = parseAdditive();
        left = new BinaryExpr(op, left, right);
    }
    return left;
}

Expression* Parser::parseAdditive() {
    Expression* left = parseMultiplicative();
    while (lexer.current().kind == Kind::tok_operator) {
        Operator op = std::get<Operator>(lexer.current().value);
        if (op != Operator::Plus && op != Operator::Minus)
            break;
        lexer.next();
        Expression* right = parseMultiplicative();
        left = new BinaryExpr(op, left, right);
    }
    return left;
}

Expression* Parser::parseMultiplicative() {
    Expression* left = parseUnary();
    while (lexer.current().kind == Kind::tok_operator) {
        Operator op = std::get<Operator>(lexer.current().value);
        if (op != Operator::Multiply && op != Operator::Divide)
            break;
        lexer.next();
        Expression* right = parseUnary();
        left = new BinaryExpr(op, left, right);
    }
    return left;
}

Expression* Parser::parseUnary() {
    if (lexer.current().kind == Kind::tok_operator) {
        Operator op = std::get<Operator>(lexer.current().value);
        if (op == Operator::Not || op == Operator::Minus) {
            lexer.next();
            Expression* operand = parseUnary();
            return new UnaryExpr(op, operand);
        }
    }
    return parsePostfix();
}

Expression* Parser::parsePostfix() {
    Expression* expr = parsePrimary();
    while (lexer.current().kind == Kind::tok_lparen) {
        lexer.next();
        std::vector<Expression*> args;
        while (lexer.current().kind != Kind::tok_rparen) {
            args.push_back(parseExpression());
            if (lexer.current().kind == Kind::tok_comma) {
                lexer.next();
            } else if (lexer.current().kind != Kind::tok_rparen) {
                error("Expected ',' or ')' in function call");
            }
        }
        consume(Kind::tok_rparen, "Expected ')' after function arguments");
        if (auto* ident = dynamic_cast<IdentifierExpr*>(expr)) {
            expr = new CallExpr(ident->name, args);
        } else {
            error("Can only call identifiers");
        }
    }
    return expr;
}

Expression* Parser::parsePrimary() {
    if (lexer.current().kind == Kind::tok_identifier) {
        std::string name = std::get<std::string>(lexer.current().value);
        lexer.next();
        return new IdentifierExpr(name);
    } else if (lexer.current().kind == Kind::tok_int_literal) {
        int64_t value = std::get<int64_t>(lexer.current().value);
        lexer.next();
        return new IntExpr(static_cast<int>(value));
    } else if (lexer.current().kind == Kind::tok_float_literal) {
        double value = std::get<double>(lexer.current().value);
        lexer.next();
        return new FloatExpr(static_cast<float>(value));
    } else if (lexer.current().kind == Kind::tok_bool_literal) {
        bool value = std::get<bool>(lexer.current().value);
        lexer.next();
        return new BoolExpr(value);
    } else if (lexer.current().kind == Kind::tok_char_literal) {
        char value = std::get<char>(lexer.current().value);
        lexer.next();
        return new CharExpr(value);
    } else if (lexer.current().kind == Kind::tok_string_literal) {
        std::string value = std::get<std::string>(lexer.current().value);
        lexer.next();
        return new StringExpr(value);
    } else if (lexer.current().kind == Kind::tok_lparen) {
        lexer.next();
        Expression* expr = parseExpression();
        consume(Kind::tok_rparen, "Expected ')' after expression");
        return expr;
    }
    error("Unexpected token in expression");
}

static void indent(int depth) {
    for (int i = 0; i < depth; ++i)
        std::cout << "  ";
}

void printExpr(Expression* expr, int depth);

void printBinary(BinaryExpr* expr, int depth) {
    indent(depth);
    std::cout << "BinaryExpr " << to_string(expr->op) << "\n";
    printExpr(expr->left, depth + 1);
    printExpr(expr->right, depth + 1);
}

void printUnary(UnaryExpr* expr, int depth) {
    indent(depth);
    std::cout << "UnaryExpr " << to_string(expr->op) << "\n";
    printExpr(expr->operand, depth + 1);
}

void printExpr(Expression* expr, int depth) {
    if (!expr) {
        indent(depth);
        std::cout << "<null>\n";
        return;
    }
    if (auto* e = dynamic_cast<IntExpr*>(expr)) {
        indent(depth);
        std::cout << "IntLiteral " << e->value << "\n";
    }
    else if (auto* e = dynamic_cast<FloatExpr*>(expr)) {
        indent(depth);
        std::cout << "FloatLiteral " << e->value << "\n";
    }
    else if (auto* e = dynamic_cast<BoolExpr*>(expr)) {
        indent(depth);
        std::cout << "BoolLiteral " << e->value << "\n";
    }
    else if (auto* e = dynamic_cast<CharExpr*>(expr)) {
        indent(depth);
        std::cout << "CharLiteral '" << e->value << "'\n";
    }
    else if (auto* e = dynamic_cast<StringExpr*>(expr)) {
        indent(depth);
        std::cout << "StringLiteral \"" << e->value << "\"\n";
    }
    else if (auto* e = dynamic_cast<IdentifierExpr*>(expr)) {
        indent(depth);
        std::cout << "Identifier " << e->name << "\n";
    }
    else if (auto* e = dynamic_cast<BinaryExpr*>(expr)) {
        printBinary(e, depth);
    }
    else if (auto* e = dynamic_cast<UnaryExpr*>(expr)) {
        printUnary(e, depth);
    }
    else if (auto* e = dynamic_cast<CallExpr*>(expr)) {
        indent(depth);
        std::cout << "CallExpr " << e->callee << "\n";
        for (auto* arg : e->arguments) {
            printExpr(arg, depth + 1);
        }
    }
    else if (auto* e = dynamic_cast<AssignExpr*>(expr)) {
        indent(depth);
        std::cout << "AssignExpr " << e->name << "\n";
        printExpr(e->value, depth + 1);
    }
    else {
        indent(depth);
        std::cout << "<unknown expr>\n";
    }
}

void printStmt(Statement* stmt, int depth);

void printBlock(BlockStmt* block, int depth) {
    indent(depth);
    std::cout << "Block\n";
    for (auto* s : block->statements) {
        printStmt(s, depth + 1);
    }
}

void printStmt(Statement* stmt, int depth) {
    if (!stmt) {
        indent(depth);
        std::cout << "<null>\n";
        return;
    }
    if (auto* s = dynamic_cast<VarDeclStmt*>(stmt)) {
        indent(depth);
        std::cout << "VarDecl "
                  << type_to_string(s->kind)
                  << " "
                  << s->name << "\n";
        if (s->initializer) {
            printExpr(s->initializer, depth + 1);
        }
    }
    else if (auto* s = dynamic_cast<ExprStmt*>(stmt)) {
        indent(depth);
        std::cout << "ExprStmt\n";
        printExpr(s->expr, depth + 1);
    }
    else if (auto* s = dynamic_cast<IfStmt*>(stmt)) {
        indent(depth);
        std::cout << "IfStmt\n";
        indent(depth + 1);
        std::cout << "Condition\n";
        printExpr(s->condition, depth + 2);
        indent(depth + 1);
        std::cout << "Then\n";
        printBlock(s->thenBlock, depth + 2);
        if (s->elseBlock) {
            indent(depth + 1);
            std::cout << "Else\n";
            printBlock(s->elseBlock, depth + 2);
        }
    }
    else if (auto* s = dynamic_cast<WhileStmt*>(stmt)) {
        indent(depth);
        std::cout << "WhileStmt\n";
        indent(depth + 1);
        std::cout << "Condition\n";
        printExpr(s->condition, depth + 2);
        indent(depth + 1);
        std::cout << "Body\n";
        printBlock(s->body, depth + 2);
    }
    else if (auto* s = dynamic_cast<ForStmt*>(stmt)) {
        indent(depth);
        std::cout << "ForStmt\n";
        indent(depth + 1);
        std::cout << "Init\n";
        if (s->init) {
            printStmt(s->init, depth + 2);
        } else {
            indent(depth + 2);
            std::cout << "<none>\n";
        }
        indent(depth + 1);
        std::cout << "Condition\n";
        if (s->condition) {
            printExpr(s->condition, depth + 2);
        } else {
            indent(depth + 2);
            std::cout << "<none>\n";
        }
        indent(depth + 1);
        std::cout << "Increment\n";
        if (s->increment) {
            printExpr(s->increment, depth + 2);
        } else {
            indent(depth + 2);
            std::cout << "<none>\n";
        }
        indent(depth + 1);
        std::cout << "Body\n";
        printBlock(s->body, depth + 2);
    }
    else if (auto* s = dynamic_cast<BlockStmt*>(stmt)) {
        printBlock(s, depth);
    }
    else if (auto* s = dynamic_cast<ReturnStmt*>(stmt)) {
        indent(depth);
        std::cout << "ReturnStmt\n";
        if (s->value) {
            printExpr(s->value, depth + 1);
        }
    }
    else {
        indent(depth);
        std::cout << "<unknown stmt>\n";
    }
}

void printItem(Item* item, int depth) {
    if (auto* f = dynamic_cast<FunctionDef*>(item)) {
        indent(depth);
        std::cout << "FunctionDef " << f->name << " " << type_to_string(f->returnType) << "\n";

        indent(depth + 1);
        std::cout << "Params\n";
        for (auto& p : f->params) {
            indent(depth + 2);
            std::cout << type_to_string(p.type) << " " << p.name << "\n";
        }

        indent(depth + 1);
        std::cout << "Body\n";
        printBlock(f->body, depth + 2);
    }
    else if (auto* e = dynamic_cast<ExternDecl*>(item)) {
        indent(depth);
        std::cout << "ExternDecl " << e->name << "\n";
        for (auto& p : e->params) {
            indent(depth + 1);
            std::cout << type_to_string(p.type) << " " << p.name << "\n";
        }
    }
    else if (auto* s = dynamic_cast<Statement*>(item)) {
        printStmt(s, depth);
    }
    else {
        indent(depth);
        std::cout << "<unknown item>\n";
    }
}

void Parser::printProgram(Program* program) {
    std::cout << "Program\n";
    for (auto* item : program->items) {
        printItem(item, 1);
    }
}