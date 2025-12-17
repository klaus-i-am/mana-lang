#pragma once

#include "../frontend/AstModule.h"

namespace mana::middle {

    // Removes unreachable code after return/break/continue statements
    // and eliminates unused variables
    struct DeadCodeElimination {
        static void run(mana::frontend::AstModule* module);
    };

} // namespace mana::middle
