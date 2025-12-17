#pragma once

#include "AstNodes.h"

namespace mana::frontend {

    struct AstNode {
        NodeKind kind;
        int line = 0;
        int column = 0;

        AstNode(NodeKind k, int l, int c) : kind(k), line(l), column(c) {}
        virtual ~AstNode() = default;
    };

} // namespace mana::frontend
