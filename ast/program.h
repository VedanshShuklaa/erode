#pragma once
#include "item.h"
#include <vector>

struct Program : Node {
    std::vector<Item*> items;
};
