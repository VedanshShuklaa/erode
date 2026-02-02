#pragma once
#include "token.h"
#include <vector>
#include <string>

class Lexer {
public:
    Lexer(const char* start, const char* end);

    const Token& current() const;
    const Token& next();

private:
    const char* cur;
    const char* end;

    std::vector<Token> tokens;
    size_t current_token_index = 0;

    Token lex();
    Token lex_alpha();
    Token lex_number();
    Token lex_operator();
    Token lex_separator();
    Token lex_string();
    Token lex_char();
    Token lex_eof();

    [[noreturn]] void error(const std::string& msg);
};