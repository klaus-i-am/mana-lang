#pragma once

#include "../frontend/AstModule.h"

namespace mana::middle {

    // Inlines small functions at call sites
    // Criteria for inlining:
    // - Function body has <= 5 statements
    // - Not recursive
    // - Not marked as #[noinline]
    struct Inlining {
        static void run(mana::frontend::AstModule* module);
        
        // Threshold for maximum statements in a function to be inlined
        static constexpr int MAX_INLINE_STATEMENTS = 5;
    };

} // namespace mana::middle
