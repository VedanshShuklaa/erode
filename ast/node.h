// The base Node class for all AST nodes
#pragma once
#include <llvm/IR/Value.h>

struct Node {
    virtual ~Node() = default;
};