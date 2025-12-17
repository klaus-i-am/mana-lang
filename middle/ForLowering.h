#pragma once

#include "../frontend/AstModule.h"

namespace mana::middle {

    // Lowers AstForStmt into:
    // { init; while (cond) { body; incr; } }
    struct ForLowering {
        static void run(mana::frontend::AstModule* module);
    };

} // namespace mana::middle
