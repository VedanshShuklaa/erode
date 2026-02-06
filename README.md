# erode

`erode` is a small experimental programming language built as a learning project focused on compiler construction, language design, and Rust internals.

The primary goal of erode is to explore how modern language features work under the hood—especially ownership, borrowing, and lifetimes—by implementing a custom borrow checker inspired by Rust’s model.

## Goals

- Learn compiler and language implementation from the ground up
- Explore parsing, type checking, and intermediate representations
- Implement a custom ownership and borrow checking system
- Gain deeper understanding of Rust’s internals by re-implementing key ideas in C++

## Status

This project is **early-stage and experimental**. Expect frequent breaking changes, incomplete features, and rough edges.

## Implementation

- Written in **C++**
- Custom lexer, parser, and type system
- Hand-rolled borrow checker (work in progress)

## Motivation

Rust’s safety guarantees are powerful but complex. Building erode is a way to learn by rebuilding: understanding not just how a borrow checker is used, but how one is designed, implemented, and reasoned about.

## Build
Inside build/ directory, run

```bash
cmake ..
make
./erode
```

## Usage

```bash
./erode <input_file> <mode>

where <mode> can be one of the following:

- test-lexer // Tests the lexer
- test-parser // Tests the parser
- test-semantics // Tests the semantic analyzer
- full // Runs the full pipeline
```

## Syntax

Functions are defined using the `def` keyword.

```erode
def function_name(type1 param1, type2 param2, ... typeN paramN) -> return_type {
    // function body
}
```

Variable declarations are done using type declarations. The supported types are :

```erode
int x = 5;
char c = 'a';
bool b = true;
float f = 3.14;
string s = "hello";
```

## To be added

- Control flow
- Memory management
- Ownership
- Borrowing
- Lifetimes
- Code generation
- Threading

## License

MIT
