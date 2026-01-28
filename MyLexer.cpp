#include <iostream>
#include <fstream>
#include <string>

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
    tok_separator,
};

struct Token {
    Kind kind;
    double value;
    std::string name;
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
            return Token{Kind::tok_def, 0, ""};
        }

        if (name == "extern") {
            return Token{Kind::tok_extern, 0, ""};
        }

        if (name == "int") {
            return Token{Kind::tok_int, 0, ""};
        }

        if (name == "float") {
            return Token{Kind::tok_float, 0, ""};
        }

        if (name == "bool") {
            return Token{Kind::tok_bool, 0, ""};
        }

        if (name == "true") {
            return Token{Kind::tok_bool_literal, 1, ""};
        }

        if (name == "false") {
            return Token{Kind::tok_bool_literal, 0, ""};
        }

        if (name == "string") {
            return Token{Kind::tok_string, 0, ""};
        }

        if (name == "char") {
            return Token{Kind::tok_char, 0, ""};
        }

        return Token{Kind::tok_identifier, 0, name};
    }

    Token lex_number() {
       double val = 0;
       while (isdigit((unsigned char)*cur)) {
           val = val * 10 + (*cur - '0');
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
           val += frac;
           return Token{Kind::tok_float_literal, val, ""};
       }
       
       return Token{Kind::tok_int_literal, val, ""}; 
    }

    Token lex_operator() {
        std::string a = std::string(1, *cur);
        cur++;
        return Token{Kind::tok_operator, 0, a};
    }

    Token lex_separator() {
        std::string a = std::string(1, *cur);
        cur++;
        return Token{Kind::tok_separator, 0, a};
    }

    Token lex_eof() {
        return Token{Kind::tok_eof, 0, ""};
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

        return {Kind::tok_string_literal, 0, value};
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

        return {Kind::tok_char_literal, (double)value, ""};
    }

    Token lex() {
        if (cur == end) {
            return lex_eof();
        }
        
        if (isalpha((unsigned char)*cur)) {
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

        if (*cur == ',' || *cur == ';') {
            return lex_separator();
        }

        if(*cur == '"') {
            return lex_string();
        }

        if(*cur == '\'') {
            return lex_char();
        }

        return lex_operator();
    }
};

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
                std::cout << "INT_LITERAL " << token.value << "\n";
                break;

            case Kind::tok_float_literal:
                std::cout << "FLOAT_LITERAL " << token.value << "\n";
                break;

            case Kind::tok_identifier:
                std::cout << "IDENTIFIER " << token.name << "\n";
                break;

            case Kind::tok_operator:
                std::cout << "OPERATOR " << token.name << "\n";
                break;

            case Kind::tok_separator:
                std::cout << "SEPARATOR " << token.name << "\n";
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
                std::cout << "CHAR_LITERAL " << token.value << "\n";
                break;

            case Kind::tok_string_literal:
                std::cout << "STRING_LITERAL " << token.name << "\n";
                break;

            case Kind::tok_bool_literal:
                std::cout << "BOOL_LITERAL " << (token.value == 1 ? "true" : "false") << "\n";
                break;

            case Kind::tok_eof:
                std::cout << "EOF\n";
                break;
        }
        token = lexer.lex();
    }

    return 0;
}