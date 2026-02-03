#include "token.h"

bool isOperator(Operator op) {
    return op == Operator::Plus || op == Operator::Minus || op == Operator::Multiply || op == Operator::Divide;
}

bool isType(Kind kind) {
    return kind == Kind::tok_int || kind == Kind::tok_float || kind == Kind::tok_bool || kind == Kind::tok_char || kind == Kind::tok_string;
}

TypeKind getTypeKind(Kind kind) {
    if (kind == Kind::tok_int) return TypeKind::INT;
    if (kind == Kind::tok_float) return TypeKind::FLOAT;
    if (kind == Kind::tok_bool) return TypeKind::BOOL;
    if (kind == Kind::tok_char) return TypeKind::CHAR;
    return TypeKind::INT;
}

bool isLiteral(Kind kind) {
    return kind == Kind::tok_int_literal || kind == Kind::tok_float_literal || kind == Kind::tok_bool_literal || kind == Kind::tok_char_literal;
}