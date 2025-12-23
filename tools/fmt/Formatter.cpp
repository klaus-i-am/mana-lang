#include "Formatter.h"
#include <sstream>

namespace mana::fmt {

using namespace frontend;

Formatter::Formatter(const FormatConfig& config) : config_(config) {}

std::string Formatter::format(const AstModule* module) {
    std::ostringstream oss;
    format(module, oss);
    return oss.str();
}

void Formatter::format(const AstModule* module, std::ostream& out) {
    if (!module) return;

    // Module declaration
    if (!module->name.empty()) {
        out << "module " << module->name << ";\n\n";
    }

    // Emit declarations
    for (const auto& decl : module->decls) {
        emit_decl(decl.get(), out);
        out << "\n";
    }
}

void Formatter::emit_indent(std::ostream& out, int level) {
    for (int i = 0; i < level; i++) {
        if (config_.use_tabs) {
            out << "\t";
        } else {
            for (int j = 0; j < config_.indent_width; j++) {
                out << " ";
            }
        }
    }
}

void Formatter::emit_type(const std::string& type, std::ostream& out) {
    out << type;
}

void Formatter::emit_decl(const AstDecl* d, std::ostream& out) {
    if (auto fn = dynamic_cast<const AstFuncDecl*>(d)) {
        // Function declaration
        if (fn->is_pub) out << "pub ";
        if (fn->is_async) out << "async ";
        out << "fn " << fn->name;

        // Generic parameters
        if (!fn->type_params.empty()) {
            out << "<";
            for (size_t i = 0; i < fn->type_params.size(); i++) {
                if (i > 0) out << ", ";
                out << fn->type_params[i];
            }
            out << ">";
        }

        // Parameters
        out << "(";
        for (size_t i = 0; i < fn->params.size(); i++) {
            if (i > 0) out << ", ";
            out << fn->params[i].name;
            if (config_.space_after_colon) out << ": ";
            else out << ":";
            emit_type(fn->params[i].type_name, out);
            if (fn->params[i].has_default()) {
                out << " = ";
                emit_expr(fn->params[i].default_value.get(), out);
            }
        }
        out << ")";

        // Return type
        if (!fn->return_type.empty()) {
            out << " -> " << fn->return_type;
        }

        // Where clause
        if (!fn->constraints.empty()) {
            out << "\n";
            emit_indent(out, 1);
            out << "where ";
            for (size_t i = 0; i < fn->constraints.size(); i++) {
                if (i > 0) out << ", ";
                out << fn->constraints[i].type_param << ": ";
                for (size_t j = 0; j < fn->constraints[i].traits.size(); j++) {
                    if (j > 0) out << " + ";
                    out << fn->constraints[i].traits[j];
                }
            }
        }

        // Body
        out << " {\n";
        if (fn->body) {
            for (const auto& stmt : fn->body->statements) {
                emit_stmt(stmt.get(), out, 1);
            }
        }
        out << "}\n";

    } else if (auto sd = dynamic_cast<const AstStructDecl*>(d)) {
        if (sd->is_pub) out << "pub ";
        out << "struct " << sd->name;
        if (!sd->type_params.empty()) {
            out << "<";
            for (size_t i = 0; i < sd->type_params.size(); i++) {
                if (i > 0) out << ", ";
                out << sd->type_params[i];
            }
            out << ">";
        }
        out << " {\n";
        for (const auto& f : sd->fields) {
            emit_indent(out, 1);
            
            out << f.name;
            if (config_.space_after_colon) out << ": ";
            else out << ":";
            emit_type(f.type_name, out);
            out << ";\n";
        }
        out << "}\n";

    } else if (auto ed = dynamic_cast<const AstEnumDecl*>(d)) {
        if (ed->is_pub) out << "pub ";
        out << "enum " << ed->name << " {\n";
        for (size_t i = 0; i < ed->variants.size(); i++) {
            emit_indent(out, 1);
            out << ed->variants[i].name;
            if (ed->variants[i].is_tuple_variant()) {
                out << "(";
                for (size_t j = 0; j < ed->variants[i].tuple_types.size(); j++) {
                    if (j > 0) out << ", ";
                    out << ed->variants[i].tuple_types[j];
                }
                out << ")";
            } else if (ed->variants[i].is_struct_variant()) {
                out << " {\n";
                for (const auto& f : ed->variants[i].struct_fields) {
                    emit_indent(out, 2);
                    out << f.name << ": " << f.type_name << ";\n";
                }
                emit_indent(out, 1);
                out << "}";
            } else if (ed->variants[i].has_value) {
                out << " = " << ed->variants[i].value;
            }
            if (i + 1 < ed->variants.size() || config_.trailing_commas) {
                out << ",";
            }
            out << "\n";
        }
        out << "}\n";

    } else if (auto td = dynamic_cast<const AstTraitDecl*>(d)) {
        if (td->is_pub) out << "pub ";
        out << "trait " << td->name << " {\n";
        for (const auto& m : td->methods) {
            emit_indent(out, 1);
            out << "fn " << m.name << "(";
            for (size_t i = 0; i < m.params.size(); i++) {
                if (i > 0) out << ", ";
                out << m.params[i].name << ": " << m.params[i].type_name;
            }
            out << ")";
            if (!m.return_type.empty()) {
                out << " -> " << m.return_type;
            }
            if (m.has_default()) {
                out << " {\n";
                // TODO: emit default body
                emit_indent(out, 1);
                out << "}\n";
            } else {
                out << ";\n";
            }
        }
        out << "}\n";

    } else if (auto impl = dynamic_cast<const AstImplDecl*>(d)) {
        out << "impl ";
        if (!impl->trait_name.empty()) {
            out << impl->trait_name << " for ";
        }
        out << impl->type_name << " {\n";
        for (const auto& method : impl->methods) {
            emit_indent(out, 1);
            if (method->is_pub) out << "pub ";
            out << "fn " << method->name << "(";
            bool first = true;
            if (!method->is_static) {
                out << "self";
                first = false;
            }
            for (const auto& p : method->params) {
                if (!first) out << ", ";
                first = false;
                out << p.name << ": " << p.type_name;
            }
            out << ")";
            if (!method->return_type.empty()) {
                out << " -> " << method->return_type;
            }
            out << " {\n";
            if (method->body) {
                for (const auto& stmt : method->body->statements) {
                    emit_stmt(stmt.get(), out, 2);
                }
            }
            emit_indent(out, 1);
            out << "}\n";
        }
        out << "}\n";

    } else if (auto use = dynamic_cast<const AstUseDecl*>(d)) {
        out << "use " << use->module_path;
        if (!use->imported_names.empty()) {
            out << "::{";
            for (size_t i = 0; i < use->imported_names.size(); i++) {
                if (i > 0) out << ", ";
                out << use->imported_names[i];
            }
            out << "}";
        }
        out << ";\n";
    }
}

void Formatter::emit_stmt(const AstStmt* s, std::ostream& out, int indent) {
    if (!s) return;

    if (auto blk = dynamic_cast<const AstBlockStmt*>(s)) {
        for (const auto& stmt : blk->statements) {
            emit_stmt(stmt.get(), out, indent);
        }
    } else if (auto vd = dynamic_cast<const AstVarDeclStmt*>(s)) {
        emit_indent(out, indent);
        out << "let ";
        if (vd->is_mutable) out << "mut ";
        out << vd->name;
        if (!vd->type_name.empty()) {
            out << ": " << vd->type_name;
        }
        if (vd->init_expr) {
            out << " = ";
            emit_expr(static_cast<const AstExpr*>(vd->init_expr.get()), out);
        }
        out << ";\n";
    } else if (auto ret = dynamic_cast<const AstReturnStmt*>(s)) {
        emit_indent(out, indent);
        out << "return";
        if (ret->value) {
            out << " ";
            emit_expr(static_cast<const AstExpr*>(ret->value.get()), out);
        }
        out << ";\n";
    } else if (auto ifs = dynamic_cast<const AstIfStmt*>(s)) {
        emit_indent(out, indent);
        out << "if ";
        emit_expr(static_cast<const AstExpr*>(ifs->condition.get()), out);
        out << " {\n";
        emit_stmt(ifs->then_block.get(), out, indent + 1);
        emit_indent(out, indent);
        out << "}";
        if (ifs->else_block) {
            out << " else {\n";
            emit_stmt(ifs->else_block.get(), out, indent + 1);
            emit_indent(out, indent);
            out << "}";
        }
        out << "\n";
    } else if (auto ws = dynamic_cast<const AstWhileStmt*>(s)) {
        emit_indent(out, indent);
        out << "while ";
        emit_expr(static_cast<const AstExpr*>(ws->condition.get()), out);
        out << " {\n";
        emit_stmt(ws->body.get(), out, indent + 1);
        emit_indent(out, indent);
        out << "}\n";
    } else if (auto es = dynamic_cast<const AstExprStmt*>(s)) {
        emit_indent(out, indent);
        emit_expr(static_cast<const AstExpr*>(es->expr.get()), out);
        out << ";\n";
    } else if (auto as = dynamic_cast<const AstAssignStmt*>(s)) {
        emit_indent(out, indent);
        if (as->target_expr) {
            emit_expr(static_cast<const AstExpr*>(as->target_expr.get()), out);
        } else {
            out << as->target_name;
        }
        out << " " << as->op << " ";
        emit_expr(static_cast<const AstExpr*>(as->value.get()), out);
        out << ";\n";
    } else if (auto brk = dynamic_cast<const AstBreakStmt*>(s)) {
        emit_indent(out, indent);
        out << "break";
        if (brk->value) {
            out << " ";
            emit_expr(static_cast<const AstExpr*>(brk->value.get()), out);
        }
        out << ";\n";
    } else if (dynamic_cast<const AstContinueStmt*>(s)) {
        emit_indent(out, indent);
        out << "continue;\n";
    } else if (auto fs = dynamic_cast<const AstForStmt*>(s)) {
        emit_indent(out, indent);
        out << "for ";
        if (fs->init) emit_stmt_inline(fs->init.get(), out);
        out << "; ";
        if (fs->condition) emit_expr(static_cast<const AstExpr*>(fs->condition.get()), out);
        out << "; ";
        if (fs->increment) emit_stmt_inline(fs->increment.get(), out);
        out << " {\n";
        emit_stmt(fs->body.get(), out, indent + 1);
        emit_indent(out, indent);
        out << "}\n";
    } else if (auto fi = dynamic_cast<const AstForInStmt*>(s)) {
        emit_indent(out, indent);
        out << "for " << fi->var_name << " in ";
        emit_expr(static_cast<const AstExpr*>(fi->iterable.get()), out);
        out << " {\n";
        emit_stmt(fi->body.get(), out, indent + 1);
        emit_indent(out, indent);
        out << "}\n";
    } else if (auto ls = dynamic_cast<const AstLoopStmt*>(s)) {
        emit_indent(out, indent);
        out << "loop {\n";
        emit_stmt(ls->body.get(), out, indent + 1);
        emit_indent(out, indent);
        out << "}\n";
    } else if (auto ds = dynamic_cast<const AstDeferStmt*>(s)) {
        emit_indent(out, indent);
        out << "defer ";
        emit_stmt(ds->body.get(), out, 0);
    } else if (auto sc = dynamic_cast<const AstScopeStmt*>(s)) {
        emit_indent(out, indent);
        if (!sc->name.empty()) {
            out << sc->name << ": ";
        }
        out << "{\n";
        if (auto blk = dynamic_cast<const AstBlockStmt*>(sc->body.get())) {
            for (const auto& stmt : blk->statements) {
                emit_stmt(stmt.get(), out, indent + 1);
            }
        } else if (sc->body) {
            emit_stmt(sc->body.get(), out, indent + 1);
        }
        emit_indent(out, indent);
        out << "}\n";
    } else if (auto dest = dynamic_cast<const AstDestructureStmt*>(s)) {
        emit_indent(out, indent);
        out << "let ";
        if (dest->is_tuple) {
            out << "(";
            for (size_t i = 0; i < dest->bindings.size(); i++) {
                if (i > 0) out << ", ";
                out << dest->bindings[i].name;
            }
            out << ")";
        } else {
            out << dest->type_name << " { ";
            for (size_t i = 0; i < dest->bindings.size(); i++) {
                if (i > 0) out << ", ";
                if (!dest->bindings[i].field_name.empty() &&
                    dest->bindings[i].field_name != dest->bindings[i].name) {
                    out << dest->bindings[i].field_name << ": ";
                }
                out << dest->bindings[i].name;
            }
            out << " }";
        }
        out << " = ";
        emit_expr(static_cast<const AstExpr*>(dest->init_expr.get()), out);
        out << ";\n";
    }
}

// Emit statement content inline (no trailing semicolon/newline) - for for-loop parts
void Formatter::emit_stmt_inline(const AstStmt* s, std::ostream& out) {
    if (!s) return;

    if (auto vd = dynamic_cast<const AstVarDeclStmt*>(s)) {
        out << "let ";
        if (vd->is_mutable) out << "mut ";
        out << vd->name;
        if (!vd->type_name.empty()) {
            out << ": " << vd->type_name;
        }
        if (vd->init_expr) {
            out << " = ";
            emit_expr(static_cast<const AstExpr*>(vd->init_expr.get()), out);
        }
    } else if (auto as = dynamic_cast<const AstAssignStmt*>(s)) {
        if (as->target_expr) {
            emit_expr(static_cast<const AstExpr*>(as->target_expr.get()), out);
        } else {
            out << as->target_name;
        }
        out << " " << as->op << " ";
        emit_expr(static_cast<const AstExpr*>(as->value.get()), out);
    } else if (auto es = dynamic_cast<const AstExprStmt*>(s)) {
        emit_expr(static_cast<const AstExpr*>(es->expr.get()), out);
    }
}

void Formatter::emit_expr(const AstExpr* e, std::ostream& out) {
    if (!e) return;

    if (auto id = dynamic_cast<const AstIdentifierExpr*>(e)) {
        out << id->name;
    } else if (auto lit = dynamic_cast<const AstLiteralExpr*>(e)) {
        if (lit->is_string) out << "\"" << lit->value << "\"";
        else if (lit->is_char) out << "'" << lit->value << "'";
        else out << lit->value;
    } else if (auto bin = dynamic_cast<const AstBinaryExpr*>(e)) {
        out << "(";
        emit_expr(static_cast<const AstExpr*>(bin->left.get()), out);
        if (config_.space_around_operators) out << " " << bin->op << " ";
        else out << bin->op;
        emit_expr(static_cast<const AstExpr*>(bin->right.get()), out);
        out << ")";
    } else if (auto un = dynamic_cast<const AstUnaryExpr*>(e)) {
        out << un->op;
        emit_expr(static_cast<const AstExpr*>(un->right.get()), out);
    } else if (auto call = dynamic_cast<const AstCallExpr*>(e)) {
        out << call->func_name << "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i > 0) out << ", ";
            emit_expr(static_cast<const AstExpr*>(call->args[i].get()), out);
        }
        out << ")";
    } else if (auto ma = dynamic_cast<const AstMemberAccessExpr*>(e)) {
        emit_expr(ma->object.get(), out);
        out << "." << ma->member_name;
    } else if (auto mc = dynamic_cast<const AstMethodCallExpr*>(e)) {
        emit_expr(mc->object.get(), out);
        out << "." << mc->method_name << "(";
        for (size_t i = 0; i < mc->args.size(); i++) {
            if (i > 0) out << ", ";
            emit_expr(static_cast<const AstExpr*>(mc->args[i].get()), out);
        }
        out << ")";
    } else if (auto idx = dynamic_cast<const AstIndexExpr*>(e)) {
        emit_expr(idx->base.get(), out);
        out << "[";
        emit_expr(idx->index.get(), out);
        out << "]";
    } else if (auto slice = dynamic_cast<const AstSliceExpr*>(e)) {
        emit_expr(slice->base.get(), out);
        out << "[";
        if (slice->start) emit_expr(slice->start.get(), out);
        out << "..";
        if (slice->end) emit_expr(slice->end.get(), out);
        out << "]";
    } else if (auto arr = dynamic_cast<const AstArrayLiteralExpr*>(e)) {
        out << "[";
        for (size_t i = 0; i < arr->elements.size(); i++) {
            if (i > 0) out << ", ";
            emit_expr(static_cast<const AstExpr*>(arr->elements[i].get()), out);
        }
        out << "]";
    } else if (auto sl = dynamic_cast<const AstStructLiteralExpr*>(e)) {
        out << sl->type_name << " {\n";
        for (size_t i = 0; i < sl->fields.size(); i++) {
            out << "    ";
            if (!sl->fields[i].field_name.empty()) {
                out << sl->fields[i].field_name << ": ";
            }
            emit_expr(sl->fields[i].value.get(), out);
            if (i + 1 < sl->fields.size() || config_.trailing_commas) out << ",";
            out << "\n";
        }
        out << "}";
    } else if (auto sa = dynamic_cast<const AstScopeAccessExpr*>(e)) {
        out << sa->scope_name << "::" << sa->member_name;
    } else if (dynamic_cast<const AstSelfExpr*>(e)) {
        out << "self";
    } else if (auto me = dynamic_cast<const AstMatchExpr*>(e)) {
        out << (me->declared_as_when ? "when " : "match ");
        emit_expr(me->value.get(), out);
        out << " {\n";
        for (const auto& arm : me->arms) {
            out << "    ";
            // Emit patterns (or-patterns)
            for (size_t i = 0; i < arm.patterns.size(); i++) {
                if (i > 0) out << " | ";
                emit_expr(arm.patterns[i].get(), out);
            }
            // Emit guard
            if (arm.guard) {
                out << " if ";
                emit_expr(arm.guard.get(), out);
            }
            out << " => ";
            // Emit result
            if (arm.result) {
                emit_expr(arm.result.get(), out);
            } else if (arm.result_block) {
                out << "{\n";
                for (const auto& stmt : arm.result_block->statements) {
                    emit_stmt(stmt.get(), out, 2);
                }
                out << "    }";
            }
            out << ",\n";
        }
        out << "}";
    } else if (auto cl = dynamic_cast<const AstClosureExpr*>(e)) {
        out << "|";
        for (size_t i = 0; i < cl->params.size(); i++) {
            if (i > 0) out << ", ";
            out << cl->params[i].name;
            if (!cl->params[i].type_name.empty()) {
                out << ": " << cl->params[i].type_name;
            }
        }
        out << "| ";
        if (!cl->return_type.empty()) {
            out << "-> " << cl->return_type << " ";
        }
        if (cl->body_expr) {
            emit_expr(cl->body_expr.get(), out);
        } else if (cl->body_block) {
            out << "{\n";
            for (const auto& stmt : cl->body_block->statements) {
                emit_stmt(stmt.get(), out, 1);
            }
            out << "}";
        }
    } else if (auto te = dynamic_cast<const AstTryExpr*>(e)) {
        emit_expr(te->operand.get(), out);
        out << "?";
    } else if (auto oc = dynamic_cast<const AstOptionalChainExpr*>(e)) {
        emit_expr(oc->object.get(), out);
        out << "?." << oc->member_name;
        if (oc->is_method_call) {
            out << "(";
            for (size_t i = 0; i < oc->args.size(); i++) {
                if (i > 0) out << ", ";
                emit_expr(static_cast<const AstExpr*>(oc->args[i].get()), out);
            }
            out << ")";
        }
    } else if (auto nc = dynamic_cast<const AstNullCoalesceExpr*>(e)) {
        emit_expr(nc->option_expr.get(), out);
        out << " ?? ";
        emit_expr(nc->default_expr.get(), out);
    } else if (auto aw = dynamic_cast<const AstAwaitExpr*>(e)) {
        emit_expr(aw->operand.get(), out);
        out << ".await";
    } else if (auto rng = dynamic_cast<const AstRangeExpr*>(e)) {
        if (rng->start) emit_expr(rng->start.get(), out);
        out << (rng->inclusive ? "..=" : "..");
        if (rng->end) emit_expr(rng->end.get(), out);
    } else if (auto tp = dynamic_cast<const AstTupleExpr*>(e)) {
        out << "(";
        for (size_t i = 0; i < tp->elements.size(); i++) {
            if (i > 0) out << ", ";
            emit_expr(static_cast<const AstExpr*>(tp->elements[i].get()), out);
        }
        if (tp->elements.size() == 1) out << ",";  // Single-element tuple
        out << ")";
    } else if (auto ti = dynamic_cast<const AstTupleIndexExpr*>(e)) {
        emit_expr(ti->tuple.get(), out);
        out << "." << ti->index;
    } else if (dynamic_cast<const AstNoneExpr*>(e)) {
        out << "None";
    } else if (auto ce = dynamic_cast<const AstCastExpr*>(e)) {
        emit_expr(ce->operand.get(), out);
        out << " as " << ce->target_type;
    } else if (auto ie = dynamic_cast<const AstIfExpr*>(e)) {
        out << "if ";
        emit_expr(ie->condition.get(), out);
        out << " { ";
        emit_expr(ie->then_expr.get(), out);
        out << " } else { ";
        emit_expr(ie->else_expr.get(), out);
        out << " }";
    } else if (auto oe = dynamic_cast<const AstOrExpr*>(e)) {
        emit_expr(oe->lhs.get(), out);
        out << " or ";
        if (oe->has_default()) {
            emit_expr(oe->default_expr.get(), out);
        } else if (oe->fallback_stmt) {
            emit_stmt(oe->fallback_stmt.get(), out, 0);
        }
    } else if (auto ep = dynamic_cast<const AstEnumPattern*>(e)) {
        out << ep->enum_name << "::" << ep->variant_name;
        if (!ep->bindings.empty()) {
            out << "(";
            for (size_t i = 0; i < ep->bindings.size(); i++) {
                if (i > 0) out << ", ";
                out << ep->bindings[i];
            }
            out << ")";
        }
    } else if (auto op = dynamic_cast<const AstOptionPattern*>(e)) {
        out << op->pattern_kind;
        if (!op->binding.empty()) {
            out << "(" << op->binding << ")";
        }
    }
}

} // namespace mana::fmt
