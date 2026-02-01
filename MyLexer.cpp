#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <cassert>

// == Lexer (copied / compatible with your lexer) ==

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

static const char* to_string(Operator op) {
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

// A very small compatibility lexer (uses your interface). In a real project
// you'd include your lexer header and link against it. For the parser below
// we assume a Lexer that has: Lexer(const char* cur, const char* end); Token lex();

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
            {"<",  Operator::Less},
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

// == AST definitions ==

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void dump(int indent = 0) const = 0;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

static void print_indent(int n) { for (int i=0;i<n;i++) std::cout << "  "; }

struct TypeNode : ASTNode {
    std::string name; // int, float, bool, string, char
    TypeNode() = default;
    TypeNode(std::string n): name(std::move(n)) {}
    void dump(int indent = 0) const override {
        print_indent(indent); std::cout << "Type: " << name << "\n";
    }
};

// Expressions
struct Expr : ASTNode { };
using ExprPtr = std::unique_ptr<Expr>;

struct LiteralExpr : Expr {
    enum LKind { INT, FLOAT, BOOL, CHAR, STRING } lkind;
    std::variant<int64_t, double, bool, char, std::string> value;
    LiteralExpr(int64_t v): lkind(INT), value(v) {}
    LiteralExpr(double v): lkind(FLOAT), value(v) {}
    LiteralExpr(bool v): lkind(BOOL), value(v) {}
    LiteralExpr(char v): lkind(CHAR), value(v) {}
    LiteralExpr(const std::string &v): lkind(STRING), value(v) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Literal(";
        switch (lkind) {
            case INT:  std::cout << std::get<int64_t>(value); break;
            case FLOAT: std::cout << std::get<double>(value); break;
            case BOOL: std::cout << (std::get<bool>(value) ? "true" : "false"); break;
            case CHAR: std::cout << '\'' << std::get<char>(value) << '\''; break;
            case STRING: std::cout << '"' << std::get<std::string>(value) << '"'; break;
        }
        std::cout << ")\n";
    }
};

struct IdentifierExpr : Expr {
    std::string name;
    IdentifierExpr(std::string n): name(std::move(n)) {}
    void dump(int indent=0) const override { print_indent(indent); std::cout<<"Ident("<<name<<")\n"; }
};

struct BinaryExpr : Expr {
    Operator op;
    ExprPtr left, right;
    BinaryExpr(Operator o, ExprPtr l, ExprPtr r): op(o), left(std::move(l)), right(std::move(r)) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Binary(" << to_string(op) << ")\n";
        left->dump(indent+1);
        right->dump(indent+1);
    }
};

struct UnaryExpr : Expr {
    Operator op;
    ExprPtr operand;
    UnaryExpr(Operator o, ExprPtr e): op(o), operand(std::move(e)) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Unary(" << to_string(op) << ")\n";
        operand->dump(indent+1);
    }
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<ExprPtr> args;
    CallExpr(std::string c): callee(std::move(c)) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Call(" << callee << ")\n";
        for (auto &a : args) a->dump(indent+1);
    }
};

// Statements
struct Stmt : ASTNode { };
using StmtPtr = std::unique_ptr<Stmt>;

struct VarDeclStmt : Stmt {
    std::unique_ptr<TypeNode> type;
    std::string name;
    std::optional<ExprPtr> init;
    VarDeclStmt(std::unique_ptr<TypeNode> t, std::string n): type(std::move(t)), name(std::move(n)), init(std::nullopt) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "VarDecl(" << name << ")\n";
        type->dump(indent+1);
        if (init) {
            print_indent(indent+1); std::cout << "Init:\n";
            (*init)->dump(indent+2);
        }
    }
};

struct ExprStmt : Stmt {
    ExprPtr expr;
    ExprStmt(ExprPtr e): expr(std::move(e)) {}
    void dump(int indent=0) const override { print_indent(indent); std::cout<<"ExprStmt\n"; expr->dump(indent+1); }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Block\n";
        for (auto &s : statements) s->dump(indent+1);
    }
};

// Top-level items: FunctionDef, ExternDecl, Statement
struct Item : ASTNode { };
using ItemPtr = std::unique_ptr<Item>;

struct FunctionDef : Item {
    std::string name;
    std::vector<std::pair<std::unique_ptr<TypeNode>, std::string>> params; // (type, name)
    std::unique_ptr<BlockStmt> body;
    FunctionDef(std::string n): name(std::move(n)) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "FunctionDef(" << name << ")\n";
        print_indent(indent+1); std::cout << "Params:\n";
        for (auto &p : params) {
            p.first->dump(indent+2);
            print_indent(indent+2); std::cout << p.second << "\n";
        }
        body->dump(indent+1);
    }
};

struct ExternDecl : Item {
    std::string name;
    std::vector<std::pair<std::unique_ptr<TypeNode>, std::string>> params;
    ExternDecl(std::string n): name(std::move(n)) {}
    void dump(int indent=0) const override {
        print_indent(indent); std::cout << "Extern(" << name << ")\n";
        for (auto &p : params) {
            p.first->dump(indent+1);
            print_indent(indent+1); std::cout << p.second << "\n";
        }
    }
};

struct TopStmtItem : Item {
    std::unique_ptr<Stmt> stmt;
    TopStmtItem(std::unique_ptr<Stmt> s): stmt(std::move(s)) {}
    void dump(int indent=0) const override { print_indent(indent); std::cout<<"TopStmtItem\n"; stmt->dump(indent+1); }
};

struct Program : ASTNode {
    std::vector<ItemPtr> items;
    void dump(int indent=0) const override {
        print_indent(indent); std::cout<<"Program\n";
        for (auto &it : items) it->dump(indent+1);
    }
};

// == Parser ==

class Parser {
    Lexer lex;
    Token cur;

    [[noreturn]] void error(const std::string &msg) {
        throw std::runtime_error(std::string("Parse error: ") + msg);
    }

    void advance() { cur = lex.lex(); }

    bool accept(Kind k) {
        if (cur.kind == k) { advance(); return true; }
        return false;
    }

    void expect(Kind k, const std::string &what = "") {
        if (cur.kind != k) {
            std::string msg = "expected token " + std::to_string(k) + " but got " + std::to_string(cur.kind);
            if (!what.empty()) msg += " (" + what + ")";
            error(msg);
        }
        advance();
    }

public:
    Parser(const char *start, const char *end): lex(start, end) { advance(); }

    std::unique_ptr<Program> parseProgram() {
        auto prog = std::make_unique<Program>();
        while (cur.kind != Kind::tok_eof) {
            prog->items.push_back(parseItem());
        }
        return prog;
    }

    ItemPtr parseItem() {
        if (cur.kind == Kind::tok_def) return parseFunctionDef();
        if (cur.kind == Kind::tok_extern) return parseExternDecl();
        // otherwise a statement at top-level
        auto s = parseStatement();
        return std::make_unique<TopStmtItem>(std::move(s));
    }

    ItemPtr parseFunctionDef() {
        expect(Kind::tok_def);
        if (cur.kind != Kind::tok_identifier) error("expected function name after 'def'");
        std::string name = std::get<std::string>(cur.value);
        advance();
        expect(Kind::tok_lparen);
        auto func = std::make_unique<FunctionDef>(name);
        if (cur.kind != Kind::tok_rparen) {
            parseParamList(func->params);
        }
        expect(Kind::tok_rparen);
        // Body
        func->body = parseBlock();
        return func;
    }

    ItemPtr parseExternDecl() {
        expect(Kind::tok_extern);
        if (cur.kind != Kind::tok_identifier) error("expected name after extern");
        std::string name = std::get<std::string>(cur.value);
        advance();
        expect(Kind::tok_lparen);
        auto ext = std::make_unique<ExternDecl>(name);
        if (cur.kind != Kind::tok_rparen) {
            parseParamList(ext->params);
        }
        expect(Kind::tok_rparen);
        expect(Kind::tok_semicolon);
        return ext;
    }

    void parseParamList(std::vector<std::pair<std::unique_ptr<TypeNode>, std::string>>& out) {
        while (true) {
            auto t = parseTypeNode();
            if (cur.kind != Kind::tok_identifier) error("expected identifier in parameter list");
            std::string name = std::get<std::string>(cur.value);
            advance();
            out.emplace_back(std::move(t), name);
            if (cur.kind == Kind::tok_comma) { advance(); continue; }
            break;
        }
    }

    std::unique_ptr<BlockStmt> parseBlock() {
        expect(Kind::tok_lbrace);
        auto block = std::make_unique<BlockStmt>();
        while (cur.kind != Kind::tok_rbrace && cur.kind != Kind::tok_eof) {
            block->statements.push_back(parseStatement());
        }
        expect(Kind::tok_rbrace);
        return block;
    }

    StmtPtr parseStatement() {
        // VarDecl ::= Type IDENT ('=' Expression)? ';'
        if (isTypeToken(cur.kind)) {
            auto type = parseTypeNode();
            if (cur.kind != Kind::tok_identifier) error("expected identifier after type in declaration");
            std::string name = std::get<std::string>(cur.value);
            advance();
            auto decl = std::make_unique<VarDeclStmt>(std::move(type), name);
            if (cur.kind == Kind::tok_operator) {
                // expect '=' operator
                Operator op = std::get<Operator>(cur.value);
                if (op == Operator::Equal) {
                    advance();
                    decl->init = parseExpression();
                } else {
                    error("expected '=' in variable declaration");
                }
            }
            expect(Kind::tok_semicolon);
            return decl;
        }

        if (cur.kind == Kind::tok_lbrace) {
            return parseBlock();
        }

        // ExprStmt ::= Expression ';'
        auto e = parseExpression();
        expect(Kind::tok_semicolon);
        return std::make_unique<ExprStmt>(std::move(e));
    }

    std::unique_ptr<TypeNode> parseTypeNode() {
        if (cur.kind == Kind::tok_int) { advance(); return std::make_unique<TypeNode>("int"); }
        if (cur.kind == Kind::tok_float) { advance(); return std::make_unique<TypeNode>("float"); }
        if (cur.kind == Kind::tok_bool) { advance(); return std::make_unique<TypeNode>("bool"); }
        if (cur.kind == Kind::tok_string) { advance(); return std::make_unique<TypeNode>("string"); }
        if (cur.kind == Kind::tok_char) { advance(); return std::make_unique<TypeNode>("char"); }
        error("unknown type token");
    }

    static bool isTypeToken(Kind k) {
        return k == Kind::tok_int || k == Kind::tok_float || k == Kind::tok_bool || k == Kind::tok_string || k == Kind::tok_char;
    }

    // Expressions following the grammar
    ExprPtr parseExpression() { return parseLogicalOr(); }

    ExprPtr parseLogicalOr() {
        auto left = parseLogicalAnd();
        while (cur.kind == Kind::tok_operator && std::get<Operator>(cur.value) == Operator::OrOr) {
            Operator op = std::get<Operator>(cur.value);
            advance();
            auto right = parseLogicalAnd();
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    }

    ExprPtr parseLogicalAnd() {
        auto left = parseEquality();
        while (cur.kind == Kind::tok_operator && std::get<Operator>(cur.value) == Operator::AndAnd) {
            Operator op = std::get<Operator>(cur.value);
            advance();
            auto right = parseEquality();
            left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
        }
        return left;
    }

    ExprPtr parseEquality() {
        auto left = parseComparison();
        while (cur.kind == Kind::tok_operator) {
            Operator op = std::get<Operator>(cur.value);
            if (op == Operator::EqualEqual || op == Operator::NotEqual) {
                advance();
                auto right = parseComparison();
                left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
                continue;
            }
            break;
        }
        return left;
    }

    ExprPtr parseComparison() {
        auto left = parseAdditive();
        while (cur.kind == Kind::tok_operator) {
            Operator op = std::get<Operator>(cur.value);
            if (op == Operator::Less || op == Operator::Greater || op == Operator::LessEqual || op == Operator::GreaterEqual) {
                advance();
                auto right = parseAdditive();
                left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
                continue;
            }
            break;
        }
        return left;
    }

    ExprPtr parseAdditive() {
        auto left = parseMultiplicative();
        while (cur.kind == Kind::tok_operator) {
            Operator op = std::get<Operator>(cur.value);
            if (op == Operator::Plus || op == Operator::Minus) {
                advance();
                auto right = parseMultiplicative();
                left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
                continue;
            }
            break;
        }
        return left;
    }

    ExprPtr parseMultiplicative() {
        auto left = parseUnary();
        while (cur.kind == Kind::tok_operator) {
            Operator op = std::get<Operator>(cur.value);
            if (op == Operator::Multiply || op == Operator::Divide) {
                advance();
                auto right = parseUnary();
                left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
                continue;
            }
            break;
        }
        return left;
    }

    ExprPtr parseUnary() {
        if (cur.kind == Kind::tok_operator) {
            Operator op = std::get<Operator>(cur.value);
            if (op == Operator::Not || op == Operator::Minus) {
                advance();
                auto operand = parseUnary();
                return std::make_unique<UnaryExpr>(op, std::move(operand));
            }
        }
        return parsePrimary();
    }

    ExprPtr parsePrimary() {
        switch (cur.kind) {
            case Kind::tok_int_literal: {
                int64_t v = std::get<int64_t>(cur.value);
                advance();
                return std::make_unique<LiteralExpr>(v);
            }
            case Kind::tok_float_literal: {
                double v = std::get<double>(cur.value);
                advance();
                return std::make_unique<LiteralExpr>(v);
            }
            case Kind::tok_bool_literal: {
                bool v = std::get<bool>(cur.value);
                advance();
                return std::make_unique<LiteralExpr>(v);
            }
            case Kind::tok_char_literal: {
                char v = std::get<char>(cur.value);
                advance();
                return std::make_unique<LiteralExpr>(v);
            }
            case Kind::tok_string_literal: {
                std::string v = std::get<std::string>(cur.value);
                advance();
                return std::make_unique<LiteralExpr>(v);
            }
            case Kind::tok_identifier: {
                std::string name = std::get<std::string>(cur.value);
                advance();
                if (cur.kind == Kind::tok_lparen) {
                    // call
                    advance(); // consume '('
                    auto call = std::make_unique<CallExpr>(name);
                    if (cur.kind != Kind::tok_rparen) {
                        while (true) {
                            call->args.push_back(parseExpression());
                            if (cur.kind == Kind::tok_comma) { advance(); continue; }
                            break;
                        }
                    }
                    expect(Kind::tok_rparen);
                    return call;
                }
                return std::make_unique<IdentifierExpr>(name);
            }
            case Kind::tok_lparen: {
                advance();
                auto e = parseExpression();
                expect(Kind::tok_rparen);
                return e;
            }
            default:
                error("unexpected token in primary expression");
        }
    }
};

// == Main / test harness ==

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) { std::cerr << "Cannot open input file\n"; return 1; }
    std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    content += '\0';

    try {
        Parser parser(content.c_str(), content.c_str() + content.size());
        auto prog = parser.parseProgram();
        prog->dump();
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
