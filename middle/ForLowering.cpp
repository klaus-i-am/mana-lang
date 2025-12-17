#include "ForLowering.h"

#include <utility>

#include "../frontend/AstDeclarations.h"
#include "../frontend/AstStatements.h"

using namespace mana::frontend;

namespace mana::middle {

    static std::unique_ptr<AstStmt> lower_stmt(std::unique_ptr<AstStmt> s);

    static void lower_block(AstBlockStmt* b) {
        if (!b) return;
        for (auto& st : b->statements) {
            st = lower_stmt(std::move(st));
        }
    }

    static std::unique_ptr<AstStmt> lower_for(std::unique_ptr<AstForStmt> f) {
        // { init; while(cond) { body; incr; } }
        auto outer = std::make_unique<AstBlockStmt>(f->line, f->column);

        if (f->init) {
            outer->statements.push_back(std::move(f->init));
        }

        auto w = std::make_unique<AstWhileStmt>(f->line, f->column);
        w->condition = std::move(f->condition);

        // while body = { <original body statements...>; incr; }
        auto inner = std::make_unique<AstBlockStmt>(f->line, f->column);

        if (f->body) {
            // lower inside the original body first
            if (f->body->kind == NodeKind::BlockStmt) {
                auto* body_block = static_cast<AstBlockStmt*>(f->body.get());
                lower_block(body_block);
                for (auto& st : body_block->statements) {
                    inner->statements.push_back(std::move(st));
                }
            } else {
                // body is a single statement, wrap it
                inner->statements.push_back(lower_stmt(std::move(f->body)));
            }
        }

        if (f->increment) {
            // increment itself may contain nested for (unlikely) but keep consistent:
            inner->statements.push_back(lower_stmt(std::move(f->increment)));
        }

        w->body = std::move(inner);
        outer->statements.push_back(std::move(w));

        // lower any nested statements we just constructed
        lower_block(outer.get());
        return outer;
    }

    static std::unique_ptr<AstStmt> lower_stmt(std::unique_ptr<AstStmt> s) {
        if (!s) return nullptr;

        switch (s->kind) {
        case NodeKind::ForStmt: {
            // take ownership as AstForStmt
            std::unique_ptr<AstForStmt> f(static_cast<AstForStmt*>(s.release()));
            return lower_for(std::move(f));
        }

        case NodeKind::BlockStmt: {
            auto* b = static_cast<AstBlockStmt*>(s.get());
            lower_block(b);
            return s;
        }

        case NodeKind::IfStmt: {
            auto* i = static_cast<AstIfStmt*>(s.get());
            if (i->then_block) {
                i->then_block = lower_stmt(std::move(i->then_block));
            }
            if (i->else_block) {
                i->else_block = lower_stmt(std::move(i->else_block));
            }
            return s;
        }

        case NodeKind::WhileStmt: {
            auto* w = static_cast<AstWhileStmt*>(s.get());
            if (w->body) {
                w->body = lower_stmt(std::move(w->body));
            }
            return s;
        }

        case NodeKind::DeferStmt: {
            auto* d = static_cast<AstDeferStmt*>(s.get());
            if (d->body) {
                d->body = lower_stmt(std::move(d->body));
            }
            return s;
        }

        default:
            return s;
        }
    }

    void ForLowering::run(AstModule* module) {
        if (!module) return;

        for (auto& d : module->decls) {
            if (!d) continue;
            if (d->kind != NodeKind::FunctionDecl) continue;

            auto* fn = static_cast<AstFuncDecl*>(d.get());
            if (!fn->body) continue;

            lower_block(fn->body.get());
        }
    }

} // namespace mana::middle
