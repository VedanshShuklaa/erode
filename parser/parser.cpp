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
    consume(Kind::tok_lbrace, "Expected '{' after function parameters");
    
    BlockStmt* body = parseBlock();
    
    consume(Kind::tok_rbrace, "Expected '}' to close function body");
    
    return new FunctionDef(name, params, body, std::nullopt);
}

BlockStmt* Parser::parseBlock() {
    BlockStmt* block = new BlockStmt();
    
    while (lexer.current().kind != Kind::tok_rbrace) {
        block->statements.push_back(parseStatement());
    }
    
    return block;
}

ExternDecl* Parser::parseExtern() {
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
            
    ExternDecl* externDecl = new ExternDecl(name, params);
    return externDecl;
}

Statement* Parser::parseStatement() {
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
    return parseLogicalOr();
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