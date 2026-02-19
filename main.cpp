#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantics/semantic_analyzer.h"
#include "codegen/codegen.h"

#include <iostream>
#include <fstream>
#include <string>

void printUsage(const char* prog) {
    std::cerr << "Usage:\n"
              << "  " << prog << " <input_file> test-lexer\n"
              << "  " << prog << " <input_file> test-parser\n"
              << "  " << prog << " <input_file> test-semantics\n"
              << "  " << prog << " <input_file> codegen\n"
              << "  " << prog << " <input_file> full\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    const std::string filename = argv[1];
    const std::string mode = argv[2];

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return 1;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    content.push_back('\0');

    std::error_code EC;
    llvm::raw_fd_ostream out("output.ll", EC);
    

    try {
        Lexer lexer(content.c_str(), content.c_str() + content.size());

        if (mode == "test-lexer") {
            lexer.test_lexer();
            return 0;
        }

        Parser parser(lexer);

        std::unique_ptr<Program> program = parser.parseProgram();

        if (mode == "test-parser") {
            std::cout << "Parse successful!\n";
            std::cout << "Top-level items: " << program->items.size() << "\n";
            parser.printProgram(program.get());
            return 0;
        }

        SemanticAnalyzer analyzer;
        analyzer.analyzeProgram(program.get());

        if (mode == "test-semantics") {
            std::cout << "Semantic analysis successful!\n";
            return 0;
        }

        CodeGen* codegen = new CodeGen();
        codegen->generate(program.get());

        if (mode == "codegen") {
            codegen->dump();
            return 0;
        }

        if (mode == "full") {
            std::cout << "Full pipeline successful!\n";
            parser.printProgram(program.get());
            return 0;
        }

        if(mode == "output") {
            codegen->getModule()->print(out, nullptr);
            return 0;
        }

        std::cerr << "Unknown mode: " << mode << "\n";
        printUsage(argv[0]);
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error\n";
        return 1;
    }
}
