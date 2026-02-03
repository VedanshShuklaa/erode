#pragma once
#include <variant>
#include <string>
#include <cstdint>

enum Kind {
    tok_eof,
    tok_def,
    tok_extern,
    tok_int,
    tok_float,
    tok_int_literal,
    tok_float_literal,
    tok_char,
    tok_char_literal,
    tok_bool,
    tok_bool_literal,
    tok_string,
    tok_string_literal,
    tok_identifier,
    tok_operator,
    tok_semicolon,
    tok_comma,
    tok_lparen,
    tok_rparen,
    tok_lbrace,
    tok_rbrace,
    tok_lbracket,
    tok_rbracket,
};

enum class Operator {
    Plus,
    Minus,
    PlusPlus,
    MinusMinus,
    Multiply,
    Divide,
    EqualEqual,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    PlusEqual,
    MinusEqual,
    AndAnd,
    OrOr,
    And,
    Or,
    Not,
    Equal,
};

struct Token {
    Kind kind;
    std::variant<
        std::monostate,
        double,
        int64_t,
        bool,
        char,
        std::string,
        Operator
    > value;
};

enum class TypeKind {
    INT,
    FLOAT,
    STRING,
    BOOL,
    CHAR
};

const char* to_string(Operator op);
