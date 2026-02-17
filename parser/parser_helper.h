#pragma once
#include "../token/token.h"

inline bool isOperator(Operator t_op) {
    return t_op == Operator::Plus || t_op == Operator::Minus || t_op == Operator::Multiply || t_op == Operator::Divide;
}

inline bool isType(Kind t_kind) {
    return t_kind == Kind::tok_int || t_kind == Kind::tok_float || t_kind == Kind::tok_bool || t_kind == Kind::tok_char || t_kind == Kind::tok_string;
}

inline TypeKind getTypeKind(Kind t_kind) {
    if (t_kind == Kind::tok_int) return TypeKind::INT;
    if (t_kind == Kind::tok_float) return TypeKind::FLOAT;
    if (t_kind == Kind::tok_bool) return TypeKind::BOOL;
    if (t_kind == Kind::tok_char) return TypeKind::CHAR;
    if (t_kind == Kind::tok_string) return TypeKind::STRING;
    return TypeKind::VOID;
}

inline bool isLiteral(Kind t_kind) {
    return t_kind == Kind::tok_int_literal || t_kind == Kind::tok_float_literal || t_kind == Kind::tok_bool_literal || t_kind == Kind::tok_char_literal;
}

inline std::string type_to_string(TypeKind t_type) {
    switch (t_type) {
        case TypeKind::INT: return "int";
        case TypeKind::FLOAT: return "float";
        case TypeKind::BOOL: return "bool";
        case TypeKind::CHAR: return "char";
        case TypeKind::VOID: return "void";
        case TypeKind::STRING: return "string";
        default: return "unknown";
    }
}