#pragma once
#include "../token/token.h"
#include <vector>
#include <string>

class Lexer {
public:
    Lexer(const char* t_start, const char* t_end);

    const Token& current() const;
    const Token& next();
    void test_lexer();

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

    [[noreturn]] void error(const std::string& t_msg);
};