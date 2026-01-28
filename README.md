# erode

`erode` is a small experimental programming language built as a learning project focused on compiler construction, language design, and Rust internals.

The primary goal of erode is to explore how modern language features work under the hood—especially ownership, borrowing, and lifetimes—by implementing a custom borrow checker inspired by Rust’s model.

## Goals

- Learn compiler and language implementation from the ground up
- Explore parsing, type checking, and intermediate representations
- Implement a custom ownership and borrow checking system
- Gain deeper understanding of Rust’s internals by re-implementing key ideas

## Non-Goals

- Production readiness
- Full Rust compatibility
- Performance optimization (for now)

## Status

This project is **early-stage and experimental**. Expect frequent breaking changes, incomplete features, and rough edges.

## Implementation

- Written in **Rust**
- Custom lexer, parser, and type system
- Hand-rolled borrow checker (work in progress)

## Motivation

Rust’s safety guarantees are powerful but complex. Building erode is a way to learn by rebuilding: understanding not just how a borrow checker is used, but how one is designed, implemented, and reasoned about.

## License

MIT
