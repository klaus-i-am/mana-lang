#include "DeadCodeElimination.h"
#include "../frontend/AstDeclarations.h"
#include "../frontend/AstStatements.h"
#include "../frontend/AstExpressions.h"

using namespace mana::frontend;

namespace mana::middle {

    // Check if a statement is a terminator (return, break, continue)
    static bool is_terminator(const AstStmt* s) {
        if (!s) return false;
        switch (s->kind) {
        case NodeKind::ReturnStmt:
        case NodeKind::BreakStmt:
        case NodeKind::ContinueStmt:
            return true;
        default:
            return false;
        }
    }

    // Check if a block always terminates (returns/breaks/continues)
    static bool always_terminates(const AstStmt* s) {
        if (!s) return false;
        
        if (is_terminator(s)) return true;
        
        if (s->kind == NodeKind::BlockStmt) {
            auto* block = static_cast<const AstBlockStmt*>(s);
            for (const auto& stmt : block->statements) {
                if (always_terminates(stmt.get())) return true;
            }
            return false;
        }
        
        if (s->kind == NodeKind::IfStmt) {
            auto* if_stmt = static_cast<const AstIfStmt*>(s);
            // Both branches must terminate for the if to always terminate
            if (if_stmt->then_block && if_stmt->else_block) {
                return always_terminates(if_stmt->then_block.get()) &&
                       always_terminates(if_stmt->else_block.get());
            }
            return false;
        }
        
        return false;
    }

    // Remove unreachable statements after terminators in a block
    static void eliminate_dead_code_in_block(AstBlockStmt* block) {
        if (!block) return;
        
        bool found_terminator = false;
        auto it = block->statements.begin();
        
        while (it != block->statements.end()) {
            if (found_terminator) {
                // Remove all statements after a terminator
                it = block->statements.erase(it);
            } else {
                // Process nested blocks
                AstStmt* s = it->get();
                
                if (s->kind == NodeKind::BlockStmt) {
                    eliminate_dead_code_in_block(static_cast<AstBlockStmt*>(s));
                }
                else if (s->kind == NodeKind::IfStmt) {
                    auto* if_stmt = static_cast<AstIfStmt*>(s);
                    if (if_stmt->then_block && if_stmt->then_block->kind == NodeKind::BlockStmt) {
                        eliminate_dead_code_in_block(static_cast<AstBlockStmt*>(if_stmt->then_block.get()));
                    }
                    if (if_stmt->else_block && if_stmt->else_block->kind == NodeKind::BlockStmt) {
                        eliminate_dead_code_in_block(static_cast<AstBlockStmt*>(if_stmt->else_block.get()));
                    }
                }
                else if (s->kind == NodeKind::WhileStmt) {
                    auto* while_stmt = static_cast<AstWhileStmt*>(s);
                    if (while_stmt->body && while_stmt->body->kind == NodeKind::BlockStmt) {
                        eliminate_dead_code_in_block(static_cast<AstBlockStmt*>(while_stmt->body.get()));
                    }
                }
                else if (s->kind == NodeKind::DeferStmt) {
                    auto* defer_stmt = static_cast<AstDeferStmt*>(s);
                    if (defer_stmt->body && defer_stmt->body->kind == NodeKind::BlockStmt) {
                        eliminate_dead_code_in_block(static_cast<AstBlockStmt*>(defer_stmt->body.get()));
                    }
                }
                
                // Check if this statement is a terminator
                if (always_terminates(s)) {
                    found_terminator = true;
                }
                
                ++it;
            }
        }
    }

    // Eliminate dead code after if statements where both branches return
    static void eliminate_unreachable_after_if(AstBlockStmt* block) {
        if (!block) return;
        
        for (size_t i = 0; i < block->statements.size(); ++i) {
            AstStmt* s = block->statements[i].get();
            
            if (s->kind == NodeKind::IfStmt) {
                auto* if_stmt = static_cast<AstIfStmt*>(s);
                
                // If both branches always terminate, everything after is dead
                if (if_stmt->then_block && if_stmt->else_block &&
                    always_terminates(if_stmt->then_block.get()) &&
                    always_terminates(if_stmt->else_block.get())) {
                    
                    // Remove all statements after this if
                    while (block->statements.size() > i + 1) {
                        block->statements.pop_back();
                    }
                    break;
                }
            }
        }
    }

    void DeadCodeElimination::run(AstModule* module) {
        if (!module) return;
        
        for (auto& d : module->decls) {
            if (!d) continue;
            if (d->kind != NodeKind::FunctionDecl) continue;
            
            auto* fn = static_cast<AstFuncDecl*>(d.get());
            if (!fn->body) continue;
            
            eliminate_dead_code_in_block(fn->body.get());
            eliminate_unreachable_after_if(fn->body.get());
        }
    }

} // namespace mana::middle
