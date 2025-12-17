#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AstNodes.h"
#include "AstDecl.h" 

namespace mana::frontend {

    struct AstModule : AstNode {
        std::string name;
        std::vector<std::unique_ptr<AstDecl>> decls;

        AstModule(std::string n, int line = 0, int column = 0)
            : AstNode(NodeKind::Module, line, column),
            name(std::move(n)) {
        }
    };

} // namespace mana::frontend
