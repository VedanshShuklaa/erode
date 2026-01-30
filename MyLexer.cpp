#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <variant>
#include <unordered_map>

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


class Lexer {
    const char *cur;
    const char *end;

private:
    [[noreturn]] void error(const std::string& msg) {
        std::cerr << "Lexer error: " << msg << std::endl;
        std::exit(1);
    }

public:
    Lexer(const char *cur, const char *end) : cur(cur), end(end) {}

    Token lex_alpha() {
        std::string name;
        while (isalpha((unsigned char)*cur) || *cur == '_' || isdigit((unsigned char)*cur)) {
            name += *cur;
            cur++;
        }
        
        if (name == "def") {
            return Token{Kind::tok_def, std::monostate{}};
        }

        if (name == "extern") {
            return Token{Kind::tok_extern, std::monostate{}};
        }

        if (name == "int") {
            return Token{Kind::tok_int, std::monostate{}};
        }

        if (name == "float") {
            return Token{Kind::tok_float, std::monostate{}};
        }

        if (name == "bool") {
            return Token{Kind::tok_bool, std::monostate{}};
        }

        if (name == "true") {
            return Token{Kind::tok_bool_literal, true};
        }

        if (name == "false") {
            return Token{Kind::tok_bool_literal, false};
        }

        if (name == "string") {
            return Token{Kind::tok_string, std::monostate{}};
        }

        if (name == "char") {
            return Token{Kind::tok_char, std::monostate{}};
        }

        return Token{Kind::tok_identifier, name};
    }

    Token lex_number() {
       // integer accumulator to avoid precision issues before we know it's a float
       int64_t intval = 0;
       while (isdigit((unsigned char)*cur)) {
           intval = intval * 10 + (*cur - '0');
           cur++;
       }

       if (*cur == '.') {
           cur++;
           double frac = 0;
           double base = 0.1;
           while(isdigit((unsigned char)*cur)) {
               frac += (*cur - '0') * base;
               base *= 0.1;
               cur++;
           }
           double val = static_cast<double>(intval) + frac;
           return Token{Kind::tok_float_literal, val};
       }
       
       return Token{Kind::tok_int_literal, intval}; 
    }

    Token lex_operator() {
        static const std::unordered_map<std::string_view, Operator> ops = {
            {"==", Operator::EqualEqual},
            {"!=", Operator::NotEqual},
            {"<=", Operator::LessEqual},
            {">=", Operator::GreaterEqual},
            {"&&", Operator::AndAnd},
            {"||", Operator::OrOr},
            {"++", Operator::PlusPlus},
            {"--", Operator::MinusMinus},
            {"+=", Operator::PlusEqual},
            {"-=", Operator::MinusEqual},
            {"+",  Operator::Plus},
            {"-",  Operator::Minus},
            {"*",  Operator::Multiply},
            {"/",  Operator::Divide},
            {"=",  Operator::Equal},
            {"!",  Operator::Not},
            {"&",  Operator::And},
            {"|",  Operator::Or},
            {"<",  Operator::Less},    // added single-char < and >
            {">",  Operator::Greater},
        };

        if (cur + 1 < end) {
            std::string_view two(cur, 2);
            if (auto it = ops.find(two); it != ops.end()) {
                cur += 2;
                return {Kind::tok_operator, it->second};
            }
        }

        std::string_view one(cur, 1);
        if (auto it = ops.find(one); it != ops.end()) {
            cur++;
            return {Kind::tok_operator, it->second};
        }

        error(std::string("unknown operator: ") + *cur);
    }

    Token lex_separator() {
        std::string a = std::string(1, *cur);
        cur++;
        switch (a[0]) {
            case ',': return Token{Kind::tok_comma, std::monostate{}};
            case ';': return Token{Kind::tok_semicolon, std::monostate{}};
            case '(' : return Token{Kind::tok_lparen, std::monostate{}};
            case ')' : return Token{Kind::tok_rparen, std::monostate{}};
            case '{' : return Token{Kind::tok_lbrace, std::monostate{}};
            case '}' : return Token{Kind::tok_rbrace, std::monostate{}};
            case '[' : return Token{Kind::tok_lbracket, std::monostate{}};
            case ']' : return Token{Kind::tok_rbracket, std::monostate{}};
            default:
                return Token{Kind::tok_eof, std::monostate{}}; // shouldn't happen
        }
    }

    Token lex_eof() {
        return Token{Kind::tok_eof, std::monostate{}};
    }

    Token lex_string() {
        cur++; // skip opening "

        std::string value;

        while (*cur != '"' && *cur != '\0') {
            value += *cur;
            cur++;
        }

        if (*cur != '"')
            error("unterminated string");

        cur++; // skip closing "

        return {Kind::tok_string_literal, value};
    }


    Token lex_char() {
        cur++; // skip '

        if (*cur == '\0')
            error("unterminated char");

        char value = *cur;

        cur++;

        if (*cur != '\'')
            error("unterminated char");

        cur++; // skip closing '

        return {Kind::tok_char_literal, value};
    }

    Token lex() {
        if (cur == end) {
            return lex_eof();
        }
        
        // allow identifiers to start with underscore too
        if (isalpha((unsigned char)*cur) || *cur == '_') {
            return lex_alpha();
        }

        if (isdigit((unsigned char)*cur)) {
            return lex_number();
        }

        if (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') {
            cur++;
            return lex();
        }

        if (*cur == '\0') {
            return lex_eof();
        }

        if (*cur == ',' || *cur == ';' || *cur == '(' || *cur == ')' || *cur == '{' || *cur == '}' || *cur == '[' || *cur == ']') {
            return lex_separator();
        }

        if(*cur == '"') {
            return lex_string();
        }

        if(*cur == '\'') {
            return lex_char();
        }

        if(*cur == '#') {
            while (*cur != '\n' && *cur != '\0') {
                cur++;
            }
            return lex();
        }

        return lex_operator();
    }
};

const char* to_string(Operator op) {
    switch (op) {
        case Operator::Plus:         return "+";
        case Operator::Minus:        return "-";
        case Operator::Multiply:     return "*";
        case Operator::Divide:       return "/";
        case Operator::EqualEqual:   return "==";
        case Operator::NotEqual:     return "!=";
        case Operator::LessEqual:    return "<=";
        case Operator::GreaterEqual: return ">=";
        case Operator::AndAnd:       return "&&";
        case Operator::OrOr:         return "||";
        case Operator::PlusPlus:     return "++";
        case Operator::MinusMinus:   return "--";
        case Operator::PlusEqual:    return "+=";
        case Operator::MinusEqual:   return "-=";
        case Operator::And:          return "&";
        case Operator::Or:           return "|";
        case Operator::Not:          return "!";
        case Operator::Equal:        return "=";
        case Operator::Greater:      return ">";
        case Operator::Less:         return "<";
    }
    return "<unknown operator>";
}

int main() {
    std::ifstream file("input.txt");
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    content += '\0';
    
    Lexer lexer(content.c_str(), content.c_str() + content.size());
    Token token = lexer.lex();
    while (token.kind != Kind::tok_eof) {
        switch (token.kind) {
            case Kind::tok_def:
                std::cout << "DEF\n";
                break;

            case Kind::tok_extern:
                std::cout << "EXTERN\n";
                break;

            case Kind::tok_int:
                std::cout << "INT\n";
                break;

            case Kind::tok_float:
                std::cout << "FLOAT\n";
                break;

            case Kind::tok_int_literal:
                std::cout << "INT_LITERAL " << std::get<int64_t>(token.value) << "\n";
                break;

            case Kind::tok_float_literal:
                std::cout << "FLOAT_LITERAL " << std::get<double>(token.value) << "\n";
                break;

            case Kind::tok_identifier:
                std::cout << "IDENTIFIER " << std::get<std::string>(token.value) << "\n";
                break;

            case Kind::tok_operator:
                std::cout << "OPERATOR " << to_string(std::get<Operator>(token.value))<< "\n";
                break;

            case Kind::tok_comma:
                std::cout << "COMMA\n";
                break;

            case Kind::tok_semicolon:
                std::cout << "SEMICOLON\n";
                break;

            case Kind::tok_lparen:
                std::cout << "LPAREN\n";
                break;

            case Kind::tok_rparen:
                std::cout << "RPAREN\n";
                break;

            case Kind::tok_lbrace:
                std::cout << "LBRACE\n";
                break;

            case Kind::tok_rbrace:
                std::cout << "RBRACE\n";
                break;

            case Kind::tok_lbracket:
                std::cout << "LBRACKET\n";
                break;

            case Kind::tok_rbracket:
                std::cout << "RBRACKET\n";
                break;

            case Kind::tok_char:
                std::cout << "CHAR " << "\n";
                break;

            case Kind::tok_bool:
                std::cout << "BOOL " << "\n";
                break;

            case Kind::tok_string:
                std::cout << "STRING " << "\n";
                break;
            
            case Kind::tok_char_literal:
                std::cout << "CHAR_LITERAL " << std::get<char>(token.value) << "\n";
                break;

            case Kind::tok_string_literal:
                std::cout << "STRING_LITERAL " << std::get<std::string>(token.value) << "\n";
                break;

            case Kind::tok_bool_literal:
                std::cout << "BOOL_LITERAL " << (std::get<bool>(token.value) ? "true" : "false") << "\n";
                break;

            case Kind::tok_eof:
                std::cout << "EOF\n";
                break;
        }
        token = lexer.lex();
    }

    return 0;
}
