#pragma once
#include "lexer.h"

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) {}
private:
    Lexer& lexer;
};