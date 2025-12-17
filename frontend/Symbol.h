#pragma once
#include <string>
#include <vector>
#include "Type.h"

namespace mana::frontend {

    struct Symbol {
        std::string name;
        Type type;
        bool is_mutable = true;
        bool is_public = false;         // Visibility: true if pub
        std::string source_module;      // Module this symbol came from (empty = current module)
        std::vector<std::string> type_params;  // Generic type parameters (for functions/structs)
        std::vector<std::pair<std::string, std::vector<std::string>>> constraints;  // Type param -> required traits
    };

} // namespace mana::frontend
