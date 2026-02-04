#include "lexer/lexer.h"
#include "parser/parser.h"
#include <iostream>


//Test Parser

int main() {
    const std::string source = R"(
        extern print(int x);

        def add(int a, int b) {
            int c = a + b * 2;
            print(c);
        }
    )";

    try {
        Lexer lexer(source.c_str(), source.c_str() + source.length());
        Parser parser(lexer);

        std::unique_ptr<Program> program = parser.parseProgram();

        std::cout << "Parse successful!\n";
        std::cout << "Top-level items: " << program->items.size() << "\n";

        for (auto* item : program->items) {
            if (dynamic_cast<ExternDecl*>(item)) {
                std::cout << "Found extern declaration\n";
            } else if (dynamic_cast<FunctionDef*>(item)) {
                std::cout << "Found function definition\n";
            } else {
                std::cout << "Unknown top-level item\n";
            }
        }

    } catch (...) {
        std::cerr << "Fatal error during parsing\n";
        return 1;
    }

    return 0;
}

// Test Lexer

// int main() {
//     std::ifstream file("input.txt");
//     std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
//     content += '\0';
    
//     Lexer lexer(content.c_str(), content.c_str() + content.size());
    
//     while (lexer.current().kind != Kind::tok_eof) {
//         Token token = lexer.current();
//         switch (token.kind) {
//             case Kind::tok_def:
//                 std::cout << "DEF\n";
//                 break;

//             case Kind::tok_extern:
//                 std::cout << "EXTERN\n";
//                 break;

//             case Kind::tok_int:
//                 std::cout << "INT\n";
//                 break;

//             case Kind::tok_float:
//                 std::cout << "FLOAT\n";
//                 break;

//             case Kind::tok_int_literal:
//                 std::cout << "INT_LITERAL " << std::get<int64_t>(token.value) << "\n";
//                 break;

//             case Kind::tok_float_literal:
//                 std::cout << "FLOAT_LITERAL " << std::get<double>(token.value) << "\n";
//                 break;

//             case Kind::tok_identifier:
//                 std::cout << "IDENTIFIER " << std::get<std::string>(token.value) << "\n";
//                 break;

//             case Kind::tok_operator:
//                 std::cout << "OPERATOR " << to_string(std::get<Operator>(token.value))<< "\n";
//                 break;

//             case Kind::tok_comma:
//                 std::cout << "COMMA\n";
//                 break;

//             case Kind::tok_semicolon:
//                 std::cout << "SEMICOLON\n";
//                 break;

//             case Kind::tok_lparen:
//                 std::cout << "LPAREN\n";
//                 break;

//             case Kind::tok_rparen:
//                 std::cout << "RPAREN\n";
//                 break;

//             case Kind::tok_lbrace:
//                 std::cout << "LBRACE\n";
//                 break;

//             case Kind::tok_rbrace:
//                 std::cout << "RBRACE\n";
//                 break;

//             case Kind::tok_lbracket:
//                 std::cout << "LBRACKET\n";
//                 break;

//             case Kind::tok_rbracket:
//                 std::cout << "RBRACKET\n";
//                 break;

//             case Kind::tok_char:
//                 std::cout << "CHAR " << "\n";
//                 break;

//             case Kind::tok_bool:
//                 std::cout << "BOOL " << "\n";
//                 break;

//             case Kind::tok_string:
//                 std::cout << "STRING " << "\n";
//                 break;
            
//             case Kind::tok_char_literal:
//                 std::cout << "CHAR_LITERAL " << std::get<char>(token.value) << "\n";
//                 break;

//             case Kind::tok_string_literal:
//                 std::cout << "STRING_LITERAL " << std::get<std::string>(token.value) << "\n";
//                 break;

//             case Kind::tok_bool_literal:
//                 std::cout << "BOOL_LITERAL " << (std::get<bool>(token.value) ? "true" : "false") << "\n";
//                 break;

//             case Kind::tok_eof:
//                 std::cout << "EOF\n";
//                 break;
//         }
//         lexer.next();
//     }
//     return 0;
// }
