#include "AstPrinter.h"
#include "AstExpressions.h"
#include "AstStatements.h"
#include "AstDeclarations.h"

namespace mana::frontend {

    void AstPrinter::indent(std::ostream& out, int n) {
        for (int i = 0; i < n; ++i) out << "  ";
    }

    void AstPrinter::print(const AstModule* m, std::ostream& out) {
        out << "Module: " << m->name << "\n";
        for (auto& d : m->decls)
            print_decl(d.get(), out, 1);
    }

    void AstPrinter::print_decl(const AstDecl* d, std::ostream& out, int ind) {
        indent(out, ind);

        if (auto fn = dynamic_cast<const AstFuncDecl*>(d)) {
            if (fn->is_method()) {
                out << "Method: " << fn->receiver_type << "." << fn->name << "(";
            } else {
                out << "Function: " << fn->name << "(";
            }
            for (size_t i = 0; i < fn->params.size(); ++i) {
                if (i > 0) out << ", ";
                out << fn->params[i].name << ": " << fn->params[i].type_name;
            }
            out << ") -> " << fn->return_type << "\n";
            if (fn->body) print_stmt(fn->body.get(), out, ind + 1);
            return;
        }

        if (auto g = dynamic_cast<const AstGlobalVarDecl*>(d)) {
            out << "GlobalVar: ";
            if (g->var) {
                out << g->var->name << ": " << g->var->type_name << "\n";
            }
            return;
        }

        if (auto imp = dynamic_cast<const AstImportDecl*>(d)) {
            out << "Import: " << imp->name << "\n";
            return;
        }

        if (auto s = dynamic_cast<const AstStructDecl*>(d)) {
            out << "Struct: " << s->name << "\n";
            for (auto& field : s->fields) {
                indent(out, ind + 1);
                out << field.name << ": " << field.type_name << "\n";
            }
            return;
        }

        if (auto e = dynamic_cast<const AstEnumDecl*>(d)) {
            out << (e->declared_as_variant ? "Variant: " : "Enum: ") << e->name << "\n";
            for (auto& variant : e->variants) {
                indent(out, ind + 1);
                out << variant.name;
                if (variant.has_value) {
                    out << " = " << variant.value;
                }
                out << "\n";
            }
            return;
        }

        out << "(decl)\n";
    }

    void AstPrinter::print_stmt(const AstStmt* s, std::ostream& out, int ind) {
        if (!s) return;
        indent(out, ind);

        if (auto b = dynamic_cast<const AstBlockStmt*>(s)) {
            out << "Block\n";
            for (auto& st : b->statements)
                print_stmt(st.get(), out, ind + 1);
            return;
        }

        if (auto v = dynamic_cast<const AstVarDeclStmt*>(s)) {
            out << "VarDecl: " << v->name << ": " << v->type_name << " = ";
            if (v->init_expr) {
                out << "\n";
                print_expr(static_cast<const AstExpr*>(v->init_expr.get()), out, ind + 1);
            } else {
                out << "(none)\n";
            }
            return;
        }

        if (auto a = dynamic_cast<const AstAssignStmt*>(s)) {
            if (a->target_expr) {
                out << "Assign:\n";
                indent(out, ind + 1); out << "Target:\n";
                print_expr(static_cast<const AstExpr*>(a->target_expr.get()), out, ind + 2);
                indent(out, ind + 1); out << "Value:\n";
                if (a->value) print_expr(static_cast<const AstExpr*>(a->value.get()), out, ind + 2);
            } else {
                out << "Assign: " << a->target_name << " = \n";
                if (a->value) print_expr(static_cast<const AstExpr*>(a->value.get()), out, ind + 1);
            }
            return;
        }

        if (auto i = dynamic_cast<const AstIfStmt*>(s)) {
            out << "If\n";
            indent(out, ind + 1); out << "Condition:\n";
            if (i->condition) print_expr(static_cast<const AstExpr*>(i->condition.get()), out, ind + 2);
            indent(out, ind + 1); out << "Then:\n";
            if (i->then_block) print_stmt(i->then_block.get(), out, ind + 2);
            if (i->else_block) {
                indent(out, ind + 1); out << "Else:\n";
                print_stmt(i->else_block.get(), out, ind + 2);
            }
            return;
        }

        if (auto w = dynamic_cast<const AstWhileStmt*>(s)) {
            out << "While\n";
            indent(out, ind + 1); out << "Condition:\n";
            if (w->condition) print_expr(static_cast<const AstExpr*>(w->condition.get()), out, ind + 2);
            indent(out, ind + 1); out << "Body:\n";
            if (w->body) print_stmt(w->body.get(), out, ind + 2);
            return;
        }

        if (auto f = dynamic_cast<const AstForStmt*>(s)) {
            out << "For\n";
            if (f->init) {
                indent(out, ind + 1); out << "Init:\n";
                print_stmt(f->init.get(), out, ind + 2);
            }
            if (f->condition) {
                indent(out, ind + 1); out << "Condition:\n";
                print_expr(static_cast<const AstExpr*>(f->condition.get()), out, ind + 2);
            }
            if (f->increment) {
                indent(out, ind + 1); out << "Increment:\n";
                print_stmt(f->increment.get(), out, ind + 2);
            }
            indent(out, ind + 1); out << "Body:\n";
            if (f->body) print_stmt(f->body.get(), out, ind + 2);
            return;
        }

        if (dynamic_cast<const AstBreakStmt*>(s)) {
            out << "Break\n";
            return;
        }

        if (dynamic_cast<const AstContinueStmt*>(s)) {
            out << "Continue\n";
            return;
        }

        if (auto r = dynamic_cast<const AstReturnStmt*>(s)) {
            out << "Return";
            if (r->value) {
                out << "\n";
                print_expr(static_cast<const AstExpr*>(r->value.get()), out, ind + 1);
            } else {
                out << " (void)\n";
            }
            return;
        }

        if (auto d = dynamic_cast<const AstDeferStmt*>(s)) {
            out << "Defer\n";
            if (d->body) print_stmt(d->body.get(), out, ind + 1);
            return;
        }

        if (auto e = dynamic_cast<const AstExprStmt*>(s)) {
            out << "ExprStmt\n";
            if (e->expr) print_expr(static_cast<const AstExpr*>(e->expr.get()), out, ind + 1);
            return;
        }

        out << "(unknown stmt)\n";
    }

    void AstPrinter::print_expr(const AstExpr* e, std::ostream& out, int ind) {
        if (!e) return;
        indent(out, ind);

        if (auto l = dynamic_cast<const AstLiteralExpr*>(e)) {
            out << "Literal: " << l->value << "\n";
            return;
        }

        if (auto id = dynamic_cast<const AstIdentifierExpr*>(e)) {
            out << "Identifier: " << id->name << "\n";
            return;
        }

        if (auto b = dynamic_cast<const AstBinaryExpr*>(e)) {
            out << "BinaryExpr: " << b->op << "\n";
            indent(out, ind + 1); out << "Left:\n";
            if (b->left) print_expr(static_cast<const AstExpr*>(b->left.get()), out, ind + 2);
            indent(out, ind + 1); out << "Right:\n";
            if (b->right) print_expr(static_cast<const AstExpr*>(b->right.get()), out, ind + 2);
            return;
        }

        if (auto u = dynamic_cast<const AstUnaryExpr*>(e)) {
            out << "UnaryExpr: " << u->op << "\n";
            if (u->right) print_expr(static_cast<const AstExpr*>(u->right.get()), out, ind + 1);
            return;
        }

        if (auto c = dynamic_cast<const AstCallExpr*>(e)) {
            out << "Call: " << c->func_name << "\n";
            for (size_t i = 0; i < c->args.size(); ++i) {
                indent(out, ind + 1); out << "Arg " << i << ":\n";
                print_expr(static_cast<const AstExpr*>(c->args[i].get()), out, ind + 2);
            }
            return;
        }

        if (auto m = dynamic_cast<const AstMethodCallExpr*>(e)) {
            out << "MethodCall: ." << m->method_name << "\n";
            indent(out, ind + 1); out << "Object:\n";
            print_expr(m->object.get(), out, ind + 2);
            for (size_t i = 0; i < m->args.size(); ++i) {
                indent(out, ind + 1); out << "Arg " << i << ":\n";
                print_expr(static_cast<const AstExpr*>(m->args[i].get()), out, ind + 2);
            }
            return;
        }

        if (auto idx = dynamic_cast<const AstIndexExpr*>(e)) {
            out << "IndexExpr\n";
            indent(out, ind + 1); out << "Base:\n";
            print_expr(idx->base.get(), out, ind + 2);
            indent(out, ind + 1); out << "Index:\n";
            print_expr(idx->index.get(), out, ind + 2);
            return;
        }

        if (auto arr = dynamic_cast<const AstArrayLiteralExpr*>(e)) {
            out << "ArrayLiteral [" << arr->elements.size() << " elements]\n";
            for (size_t i = 0; i < arr->elements.size(); ++i) {
                indent(out, ind + 1); out << "[" << i << "]:\n";
                print_expr(arr->elements[i].get(), out, ind + 2);
            }
            return;
        }

        if (auto m = dynamic_cast<const AstMemberAccessExpr*>(e)) {
            out << "MemberAccess: ." << m->member_name << "\n";
            indent(out, ind + 1); out << "Object:\n";
            print_expr(m->object.get(), out, ind + 2);
            return;
        }

        if (auto s = dynamic_cast<const AstStructLiteralExpr*>(e)) {
            out << "StructLiteral: " << s->type_name;
            if (s->is_named) out << " (named)";
            out << "\n";
            for (size_t i = 0; i < s->fields.size(); ++i) {
                indent(out, ind + 1);
                if (s->is_named) {
                    out << s->fields[i].field_name << ":\n";
                } else {
                    out << "[" << i << "]:\n";
                }
                print_expr(s->fields[i].value.get(), out, ind + 2);
            }
            return;
        }

        if (auto s = dynamic_cast<const AstScopeAccessExpr*>(e)) {
            out << "ScopeAccess: " << s->scope_name << "::" << s->member_name << "\n";
            return;
        }

        if (dynamic_cast<const AstSelfExpr*>(e)) {
            out << "Self\n";
            return;
        }

        if (auto m = dynamic_cast<const AstMatchExpr*>(e)) {
            out << (m->declared_as_when ? "When" : "Match") << "\n";
            indent(out, ind + 1); out << "Value:\n";
            print_expr(m->value.get(), out, ind + 2);
            for (size_t i = 0; i < m->arms.size(); ++i) {
                indent(out, ind + 1); out << "Arm " << i << ":\n";
                for (const auto& pat : m->arms[i].patterns) {
                    indent(out, ind + 2); out << "Pattern:\n";
                    print_expr(pat.get(), out, ind + 3);
                }
                if (m->arms[i].guard) {
                    indent(out, ind + 2); out << "Guard:\n";
                    print_expr(m->arms[i].guard.get(), out, ind + 3);
                }
                indent(out, ind + 2); out << "Result:\n";
                print_expr(m->arms[i].result.get(), out, ind + 3);
            }
            return;
        }

        out << "(unknown expr)\n";
    }

} // namespace mana::frontend
