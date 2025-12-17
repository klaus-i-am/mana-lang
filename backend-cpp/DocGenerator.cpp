#include "DocGenerator.h"

namespace mana::backend {

    using namespace mana::frontend;

    std::string DocGenerator::generate(const AstModule& mod) {
        out_.str("");
        out_.clear();
        emit_module(mod);
        return out_.str();
    }

    void DocGenerator::emit_module(const AstModule& mod) {
        out_ << "# Module: " << mod.name << "\n\n";

        // Collect declarations by type
        std::vector<const AstFuncDecl*> functions;
        std::vector<const AstStructDecl*> structs;
        std::vector<const AstEnumDecl*> enums;
        std::vector<const AstTraitDecl*> traits;
        std::vector<const AstTypeAliasDecl*> type_aliases;

        for (const auto& decl : mod.decls) {
            if (auto fn = dynamic_cast<const AstFuncDecl*>(decl.get())) {
                functions.push_back(fn);
            } else if (auto s = dynamic_cast<const AstStructDecl*>(decl.get())) {
                structs.push_back(s);
            } else if (auto e = dynamic_cast<const AstEnumDecl*>(decl.get())) {
                enums.push_back(e);
            } else if (auto t = dynamic_cast<const AstTraitDecl*>(decl.get())) {
                traits.push_back(t);
            } else if (auto ta = dynamic_cast<const AstTypeAliasDecl*>(decl.get())) {
                type_aliases.push_back(ta);
            }
        }

        // Emit type aliases
        if (!type_aliases.empty()) {
            out_ << "## Type Aliases\n\n";
            for (const auto* t : type_aliases) {
                emit_type_alias(*t);
            }
        }

        // Emit structs
        if (!structs.empty()) {
            out_ << "## Structs\n\n";
            for (const auto* s : structs) {
                emit_struct(*s);
            }
        }

        // Emit enums
        if (!enums.empty()) {
            out_ << "## Enums\n\n";
            for (const auto* e : enums) {
                emit_enum(*e);
            }
        }

        // Emit traits
        if (!traits.empty()) {
            out_ << "## Traits\n\n";
            for (const auto* t : traits) {
                emit_trait(*t);
            }
        }

        // Emit functions
        if (!functions.empty()) {
            out_ << "## Functions\n\n";
            for (const auto* fn : functions) {
                emit_function(*fn);
            }
        }
    }

    void DocGenerator::emit_function(const AstFuncDecl& fn) {
        // Function header
        out_ << "### ";
        if (fn.is_pub) out_ << "`pub` ";
        if (fn.is_async) out_ << "`async` ";
        out_ << "`fn " << fn.name << "`\n\n";

        // Signature
        out_ << "```mana\n";
        if (fn.is_pub) out_ << "pub ";
        if (fn.is_async) out_ << "async ";
        out_ << "fn ";
        if (!fn.receiver_type.empty()) {
            out_ << fn.receiver_type << ".";
        }
        out_ << fn.name;

        // Type parameters
        if (!fn.type_params.empty()) {
            out_ << "<";
            for (size_t i = 0; i < fn.type_params.size(); ++i) {
                if (i > 0) out_ << ", ";
                out_ << fn.type_params[i];
            }
            out_ << ">";
        }

        // Parameters
        out_ << "(";
        for (size_t i = 0; i < fn.params.size(); ++i) {
            if (i > 0) out_ << ", ";
            out_ << fn.params[i].name << ": " << fn.params[i].type_name;
            if (fn.params[i].has_default()) {
                out_ << " = ...";
            }
        }
        out_ << ")";

        // Return type
        if (!fn.return_type.empty() && fn.return_type != "void") {
            out_ << " -> " << fn.return_type;
        }

        // Where clause
        if (!fn.constraints.empty()) {
            out_ << "\n    where ";
            for (size_t i = 0; i < fn.constraints.size(); ++i) {
                if (i > 0) out_ << ", ";
                out_ << fn.constraints[i].type_param << ": ";
                for (size_t j = 0; j < fn.constraints[i].traits.size(); ++j) {
                    if (j > 0) out_ << " + ";
                    out_ << fn.constraints[i].traits[j];
                }
            }
        }

        out_ << "\n```\n\n";

        // Documentation comment
        if (fn.has_doc()) {
            out_ << fn.doc_comment << "\n\n";
        }

        // Parameters table
        if (!fn.params.empty()) {
            out_ << "**Parameters:**\n\n";
            out_ << "| Name | Type | Default |\n";
            out_ << "|------|------|--------|\n";
            for (const auto& p : fn.params) {
                out_ << "| `" << p.name << "` | `" << p.type_name << "` | ";
                out_ << (p.has_default() ? "yes" : "-") << " |\n";
            }
            out_ << "\n";
        }

        // Return type
        if (!fn.return_type.empty() && fn.return_type != "void") {
            out_ << "**Returns:** `" << fn.return_type << "`\n\n";
        }

        out_ << "---\n\n";
    }

    void DocGenerator::emit_struct(const AstStructDecl& s) {
        out_ << "### ";
        if (s.is_pub) out_ << "`pub` ";
        out_ << "`struct " << s.name << "`\n\n";

        // Signature
        out_ << "```mana\n";
        if (s.is_pub) out_ << "pub ";
        out_ << "struct " << s.name;

        if (!s.type_params.empty()) {
            out_ << "<";
            for (size_t i = 0; i < s.type_params.size(); ++i) {
                if (i > 0) out_ << ", ";
                out_ << s.type_params[i];
            }
            out_ << ">";
        }

        out_ << " {\n";
        for (const auto& f : s.fields) {
            out_ << "    " << f.name << ": " << f.type_name;
            if (f.default_value) out_ << " = ...";
            out_ << ",\n";
        }
        out_ << "}\n```\n\n";

        // Documentation comment
        if (s.has_doc()) {
            out_ << s.doc_comment << "\n\n";
        }

        // Fields table
        if (!s.fields.empty()) {
            out_ << "**Fields:**\n\n";
            out_ << "| Name | Type | Default |\n";
            out_ << "|------|------|--------|\n";
            for (const auto& f : s.fields) {
                out_ << "| `" << f.name << "` | `" << f.type_name << "` | ";
                out_ << (f.default_value ? "yes" : "-") << " |\n";
            }
            out_ << "\n";
        }

        out_ << "---\n\n";
    }

    void DocGenerator::emit_enum(const AstEnumDecl& e) {
        out_ << "### ";
        if (e.is_pub) out_ << "`pub` ";
        out_ << "`enum " << e.name << "`\n\n";

        // Signature
        out_ << "```mana\n";
        if (e.is_pub) out_ << "pub ";
        out_ << "enum " << e.name << " {\n";
        for (const auto& v : e.variants) {
            out_ << "    " << v.name;
            if (v.is_tuple_variant()) {
                out_ << "(";
                for (size_t i = 0; i < v.tuple_types.size(); ++i) {
                    if (i > 0) out_ << ", ";
                    out_ << v.tuple_types[i];
                }
                out_ << ")";
            } else if (v.is_struct_variant()) {
                out_ << " { ";
                for (size_t i = 0; i < v.struct_fields.size(); ++i) {
                    if (i > 0) out_ << ", ";
                    out_ << v.struct_fields[i].name << ": " << v.struct_fields[i].type_name;
                }
                out_ << " }";
            } else if (v.has_value) {
                out_ << " = " << v.value;
            }
            out_ << ",\n";
        }
        out_ << "}\n```\n\n";

        // Documentation comment
        if (e.has_doc()) {
            out_ << e.doc_comment << "\n\n";
        }

        // Variants table
        if (!e.variants.empty()) {
            out_ << "**Variants:**\n\n";
            out_ << "| Name | Data |\n";
            out_ << "|------|------|\n";
            for (const auto& v : e.variants) {
                out_ << "| `" << v.name << "` | ";
                if (v.is_tuple_variant()) {
                    out_ << "tuple(";
                    for (size_t i = 0; i < v.tuple_types.size(); ++i) {
                        if (i > 0) out_ << ", ";
                        out_ << v.tuple_types[i];
                    }
                    out_ << ")";
                } else if (v.is_struct_variant()) {
                    out_ << "struct";
                } else if (v.has_value) {
                    out_ << "= " << v.value;
                } else {
                    out_ << "-";
                }
                out_ << " |\n";
            }
            out_ << "\n";
        }

        out_ << "---\n\n";
    }

    void DocGenerator::emit_trait(const AstTraitDecl& t) {
        out_ << "### ";
        if (t.is_pub) out_ << "`pub` ";
        out_ << "`trait " << t.name << "`\n\n";

        // Signature
        out_ << "```mana\n";
        if (t.is_pub) out_ << "pub ";
        out_ << "trait " << t.name << " {\n";

        // Associated types
        for (const auto& at : t.associated_types) {
            out_ << "    type " << at.name << ";\n";
        }

        // Methods
        for (const auto& m : t.methods) {
            out_ << "    fn " << m.name << "(";
            for (size_t i = 0; i < m.params.size(); ++i) {
                if (i > 0) out_ << ", ";
                out_ << m.params[i].name << ": " << m.params[i].type_name;
            }
            out_ << ")";
            if (!m.return_type.empty() && m.return_type != "void") {
                out_ << " -> " << m.return_type;
            }
            if (m.has_default()) {
                out_ << " { ... }";
            }
            out_ << "\n";
        }

        out_ << "}\n```\n\n";

        // Documentation comment
        if (t.has_doc()) {
            out_ << t.doc_comment << "\n\n";
        }

        // Methods table
        if (!t.methods.empty()) {
            out_ << "**Methods:**\n\n";
            out_ << "| Name | Signature | Default |\n";
            out_ << "|------|-----------|--------|\n";
            for (const auto& m : t.methods) {
                out_ << "| `" << m.name << "` | `fn(";
                for (size_t i = 0; i < m.params.size(); ++i) {
                    if (i > 0) out_ << ", ";
                    out_ << m.params[i].type_name;
                }
                out_ << ")";
                if (!m.return_type.empty() && m.return_type != "void") {
                    out_ << " -> " << m.return_type;
                }
                out_ << "` | " << (m.has_default() ? "yes" : "-") << " |\n";
            }
            out_ << "\n";
        }

        out_ << "---\n\n";
    }

    void DocGenerator::emit_type_alias(const AstTypeAliasDecl& t) {
        out_ << "### ";
        if (t.is_pub) out_ << "`pub` ";
        out_ << "`type " << t.alias_name << "`\n\n";

        // Signature
        out_ << "```mana\n";
        if (t.is_pub) out_ << "pub ";
        out_ << "type " << t.alias_name << " = " << t.target_type << ";\n";
        out_ << "```\n\n";

        // Documentation comment
        if (t.has_doc()) {
            out_ << t.doc_comment << "\n\n";
        }

        out_ << "---\n\n";
    }

    std::string DocGenerator::escape_markdown(const std::string& s) {
        std::string result;
        for (char c : s) {
            if (c == '*' || c == '_' || c == '`' || c == '[' || c == ']' ||
                c == '(' || c == ')' || c == '#' || c == '+' || c == '-' ||
                c == '!' || c == '\\') {
                result += '\\';
            }
            result += c;
        }
        return result;
    }

} // namespace mana::backend
