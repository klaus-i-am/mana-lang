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
    } else if (dynamic_cast<const AstBreakStmt*>(s)) {
        emit_indent(out, indent);
        out << "break;\n";
    } else if (dynamic_cast<const AstContinueStmt*>(s)) {
        emit_indent(out, indent);
        out << "continue;\n";
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
    }
}

} // namespace mana::fmt
