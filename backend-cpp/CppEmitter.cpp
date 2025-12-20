#include "CppEmitter.h"
#include <sstream>
#include <unordered_set>
#include <variant>

namespace mana::backend {

using namespace mana::frontend;

static int match_counter = 0;
static int try_counter = 0;
static int destructure_counter = 0;
static int while_let_counter = 0;

static std::string map_type(const std::string& mana_type) {
    if (mana_type.empty() || mana_type == "void") return "void";
    if (mana_type == "i32") return "int32_t";
    if (mana_type == "int") return "int64_t";  // int alias (vNext) -> i64
    if (mana_type == "i64") return "int64_t";
    if (mana_type == "i8") return "int8_t";
    if (mana_type == "i16") return "int16_t";
    if (mana_type == "u8") return "uint8_t";
    if (mana_type == "u16") return "uint16_t";
    if (mana_type == "u32") return "uint32_t";
    if (mana_type == "u64") return "uint64_t";
    if (mana_type == "f32") return "float";
    if (mana_type == "float") return "double";  // float alias (vNext) -> f64
    if (mana_type == "f64") return "double";
    if (mana_type == "bool") return "bool";
    if (mana_type == "string" || mana_type == "String" || mana_type == "str") return "std::string";
    if (mana_type == "char") return "char";
    size_t bracket = mana_type.find('<');
    if (bracket != std::string::npos) {
        std::string base = mana_type.substr(0, bracket);
        std::string inner = mana_type.substr(bracket + 1, mana_type.size() - bracket - 2);
        std::vector<std::string> params;
        int depth = 0;
        size_t start = 0;
        for (size_t i = 0; i < inner.size(); ++i) {
            if (inner[i] == '<') depth++;
            else if (inner[i] == '>') depth--;
            else if (inner[i] == ',' && depth == 0) {
                params.push_back(inner.substr(start, i - start));
                start = i + 1;
                while (start < inner.size() && inner[start] == ' ') start++;
            }
        }
        params.push_back(inner.substr(start));
        std::string mapped_inner;
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) mapped_inner += ", ";
            mapped_inner += map_type(params[i]);
        }
        if (base == "Result" || base == "Option" || base == "Vec" || base == "HashMap") {
            return "mana::" + base + "<" + mapped_inner + ">";
        }
        return base + "<" + mapped_inner + ">";
    }
    if (mana_type.front() == '[') {
        size_t semi = mana_type.find(';');
        if (semi != std::string::npos) {
            std::string elem = mana_type.substr(1, semi - 1);
            std::string count = mana_type.substr(semi + 1);
            count = count.substr(0, count.size() - 1);
            while (!count.empty() && count.front() == ' ') count = count.substr(1);
            while (!count.empty() && count.back() == ' ') count.pop_back();
            return "std::array<" + map_type(elem) + ", " + count + ">";
        }
    }
    if (mana_type.front() == '(' && mana_type.back() == ')') {
        std::string inner = mana_type.substr(1, mana_type.size() - 2);
        std::vector<std::string> parts;
        int depth = 0;
        size_t start = 0;
        for (size_t i = 0; i < inner.size(); ++i) {
            if (inner[i] == '(' || inner[i] == '<') depth++;
            else if (inner[i] == ')' || inner[i] == '>') depth--;
            else if (inner[i] == ',' && depth == 0) {
                std::string part = inner.substr(start, i - start);
                while (!part.empty() && part.front() == ' ') part = part.substr(1);
                while (!part.empty() && part.back() == ' ') part.pop_back();
                parts.push_back(part);
                start = i + 1;
            }
        }
        std::string last = inner.substr(start);
        while (!last.empty() && last.front() == ' ') last = last.substr(1);
        while (!last.empty() && last.back() == ' ') last.pop_back();
        parts.push_back(last);
        std::string result = "std::tuple<";
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) result += ", ";
            result += map_type(parts[i]);
        }
        result += ">";
        return result;
    }
    // Handle dyn TraitName - trait object (dynamic dispatch)
    if (mana_type.rfind("dyn ", 0) == 0) {
        std::string trait_name = mana_type.substr(4);  // Skip "dyn "
        return "std::unique_ptr<I" + trait_name + ">";
    }
    // Handle &dyn TraitName - reference to trait object
    if (mana_type.rfind("&dyn ", 0) == 0) {
        std::string trait_name = mana_type.substr(5);
        return "I" + trait_name + "*";
    }
    // Handle Box<dyn TraitName>
    if (mana_type.rfind("Box<dyn ", 0) == 0) {
        std::string trait_name = mana_type.substr(8, mana_type.size() - 9);
        return "std::unique_ptr<I" + trait_name + ">";
    }
    return mana_type;
}

static std::string escape_cpp_string(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            case '\r': result += "\\r"; break;
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            case '\0': result += "\\0"; break;
            default: result += c; break;
        }
    }
    return result;
}

void CppEmitter::indent(std::ostream& out, int n) {
    for (int i = 0; i < n; ++i) out << "    ";
}

// Helper to emit closure capture list
void CppEmitter::emit_capture_list(const AstClosureExpr* cl, std::ostream& out) {
    out << "[";
    if (cl->has_explicit_captures && !cl->captures.empty()) {
        for (size_t i = 0; i < cl->captures.size(); ++i) {
            if (i > 0) out << ", ";
            const auto& cap = cl->captures[i];
            switch (cap.mode) {
                case CaptureMode::ByRef:
                    out << "&" << cap.name;
                    break;
                case CaptureMode::ByValue:
                    out << cap.name;
                    break;
                case CaptureMode::ByMove:
                    out << cap.name << " = std::move(" << cap.name << ")";
                    break;
            }
        }
    } else {
        out << (cl->captures_by_ref ? "&" : "=");
    }
    out << "]";
}

void CppEmitter::emit_expr(const AstExpr* e, std::ostream& out) {
    if (!e) return;
    switch (e->kind) {
        case NodeKind::IdentifierExpr: {
            auto id = static_cast<const AstIdentifierExpr*>(e);
            out << id->name;
            break;
        }
        case NodeKind::LiteralExpr: {
            auto lit = static_cast<const AstLiteralExpr*>(e);
            if (lit->is_string) out << "\"" << escape_cpp_string(lit->value) << "\"";
            else if (lit->is_char) out << "'" << escape_cpp_string(lit->value) << "'";
            else out << lit->value;
            break;
        }
        case NodeKind::BinaryExpr: {
            auto bin = static_cast<const AstBinaryExpr*>(e);
            out << "(";
            emit_expr(static_cast<const AstExpr*>(bin->left.get()), out);
            out << " " << bin->op << " ";
            emit_expr(static_cast<const AstExpr*>(bin->right.get()), out);
            out << ")";
            break;
        }
        case NodeKind::UnaryExpr: {
            auto un = static_cast<const AstUnaryExpr*>(e);
            out << "(" << un->op;
            emit_expr(static_cast<const AstExpr*>(un->right.get()), out);
            out << ")";
            break;
        }
        case NodeKind::CallExpr: {
            auto call = static_cast<const AstCallExpr*>(e);
            std::string fname = call->func_name;
            // Handle static method calls like HashMap::new(), Point::new()
            size_t scope_pos = fname.find("::");
            if (scope_pos != std::string::npos) {
                std::string type_name = fname.substr(0, scope_pos);
                std::string method = fname.substr(scope_pos + 2);
                if (method == "new") {
                    // Built-in mana types with special constructors
                    if (type_name == "HashMap" || type_name == "Vec") {
                        out << "mana::" << type_name << "<>{}";
                        break;
                    }
                }
                // Transform Type::method to Type_method for user-defined impl methods
                fname = type_name + "_" + method;
            }
            if (fname == "println") fname = "mana::println";
            else if (fname == "print") fname = "mana::print";
            else if (fname == "Some") fname = "mana::Some";
            else if (fname == "None") { out << "mana::None"; break; }
            else if (fname == "Ok" || fname == "ok") fname = "mana::Ok";
            else if (fname == "Err" || fname == "err") fname = "mana::Err";
            else if (fname == "some") fname = "mana::Some";
            else if (fname == "assert") fname = "mana::assert_true";
            else if (fname == "format") fname = "mana::format";
            else if (fname == "read_file") fname = "mana::read_file";
            else if (fname == "write_file") fname = "mana::write_file";
            else if (fname == "append_file") fname = "mana::append_file";
            else if (fname == "file_exists") fname = "mana::file_exists";
            else if (fname == "delete_file") fname = "mana::delete_file";
            else if (fname == "read_lines") fname = "mana::read_lines";
            else if (fname == "first") fname = "mana::first";
            else if (fname == "last") fname = "mana::last";
            else if (fname == "concat") fname = "mana::concat";
            else if (fname == "flatten") fname = "mana::flatten";
            else if (fname == "zip") fname = "mana::zip";
            else if (fname == "unzip") fname = "mana::unzip";
            else if (fname == "repeat") fname = "mana::repeat";
            // Time functions
            else if (fname == "time_now_ms") fname = "mana::time_now_ms";
            else if (fname == "time_now_secs") fname = "mana::time_now_secs";
            else if (fname == "sleep_ms") fname = "mana::sleep_ms";
            // Random functions
            else if (fname == "random_int") fname = "mana::random_int";
            // Path functions
            else if (fname == "path_join") fname = "mana::path_join";
            else if (fname == "path_parent") fname = "mana::path_parent";
            else if (fname == "path_filename") fname = "mana::path_filename";
            else if (fname == "path_extension") fname = "mana::path_extension";
            else if (fname == "is_directory") fname = "mana::is_directory";
            else if (fname == "cwd") fname = "mana::cwd";
            // Environment functions
            else if (fname == "env_get") fname = "mana::env_get";
            // Vec utilities
            else if (fname == "vec_sort") fname = "mana::vec_sort";
            else if (fname == "vec_reverse") fname = "mana::vec_reverse";
            else if (fname == "vec_contains") fname = "mana::vec_contains";
            // String functions
            else if (fname == "len") fname = "mana::len";
            else if (fname == "is_empty") fname = "mana::is_empty";
            else if (fname == "to_string") fname = "mana::to_string";
            else if (fname == "trim") fname = "mana::trim";
            else if (fname == "split") fname = "mana::split";
            else if (fname == "join") fname = "mana::join";
            else if (fname == "starts_with") fname = "mana::starts_with";
            else if (fname == "ends_with") fname = "mana::ends_with";
            else if (fname == "contains") fname = "mana::contains";
            else if (fname == "replace") fname = "mana::replace";
            else if (fname == "to_uppercase") fname = "mana::to_uppercase";
            else if (fname == "to_lowercase") fname = "mana::to_lowercase";
            else if (fname == "substr") fname = "mana::substr";
            else if (fname == "read_line") fname = "mana::read_line";
            else if (fname == "parse_int") fname = "mana::parse_int";
            else if (fname == "parse_float") fname = "mana::parse_float";
            // Math functions
            else if (fname == "min") fname = "mana::min";
            else if (fname == "max") fname = "mana::max";
            else if (fname == "clamp") fname = "mana::clamp";
            // Assert functions
            else if (fname == "assert_true") fname = "mana::assert_true";
            else if (fname == "abs") fname = "std::abs";
            else if (fname == "sqrt") fname = "std::sqrt";
            else if (fname == "pow") fname = "std::pow";
            else if (fname == "sin" || fname == "cos" || fname == "tan" ||
                     fname == "floor" || fname == "ceil" || fname == "round" ||
                     fname == "log" || fname == "log10" || fname == "exp") {
                fname = "std::" + fname;
            }
            out << fname << "(";
            for (size_t i = 0; i < call->args.size(); ++i) {
                if (i > 0) out << ", ";
                emit_expr(static_cast<const AstExpr*>(call->args[i].get()), out);
            }
            out << ")";
            break;
        }
        case NodeKind::MethodCallExpr: {
            auto mc = static_cast<const AstMethodCallExpr*>(e);
            // Check for string methods that need mana:: prefix
            static const std::unordered_set<std::string> string_methods = {
                "starts_with", "ends_with", "contains", "trim", "substr",
                "replace", "to_uppercase", "to_lowercase", "split",
                "repeat", "reverse", "join"
            };
            // Check if this is an impl method call
            std::string impl_method_name;
            if (!mc->object_type.empty()) {
                impl_method_name = mc->object_type + "_" + mc->method_name;
            }
            if (!impl_method_name.empty() && impl_methods_.count(impl_method_name)) {
                // Emit as impl method: TypeName_methodName(object, args...)
                out << impl_method_name << "(";
                emit_expr(mc->object.get(), out);
                for (size_t i = 0; i < mc->args.size(); ++i) {
                    out << ", ";
                    emit_expr(static_cast<const AstExpr*>(mc->args[i].get()), out);
                }
                out << ")";
            } else if (string_methods.count(mc->method_name)) {
                // Check if object is a Vec type - Vec has its own contains, reverse methods
                bool is_vec_type = mc->object_type.find("Vec") != std::string::npos;
                if (is_vec_type) {
                    // Use method call syntax for Vec
                    emit_expr(mc->object.get(), out);
                    out << "." << mc->method_name << "(";
                    for (size_t i = 0; i < mc->args.size(); ++i) {
                        if (i > 0) out << ", ";
                        emit_expr(static_cast<const AstExpr*>(mc->args[i].get()), out);
                    }
                    out << ")";
                } else {
                    // Use global mana:: function for strings
                    out << "mana::" << mc->method_name << "(";
                    emit_expr(mc->object.get(), out);
                    for (size_t i = 0; i < mc->args.size(); ++i) {
                        out << ", ";
                        emit_expr(static_cast<const AstExpr*>(mc->args[i].get()), out);
                    }
                    out << ")";
                }
            } else {
                emit_expr(mc->object.get(), out);
                out << "." << mc->method_name << "(";
                for (size_t i = 0; i < mc->args.size(); ++i) {
                    if (i > 0) out << ", ";
                    emit_expr(static_cast<const AstExpr*>(mc->args[i].get()), out);
                }
                out << ")";
            }
            break;
        }
        case NodeKind::IndexExpr: {
            auto idx = static_cast<const AstIndexExpr*>(e);
            emit_expr(idx->base.get(), out);
            out << ".at(";
            emit_expr(idx->index.get(), out);
            out << ")";
            break;
        }
        case NodeKind::SliceExpr: {
            auto sl = static_cast<const AstSliceExpr*>(e);
            if (sl->inclusive) out << "mana::slice_inclusive(";
            else out << "mana::slice(";
            emit_expr(sl->base.get(), out);
            out << ", ";
            if (sl->start) emit_expr(sl->start.get(), out);
            else out << "0";
            out << ", ";
            if (sl->end) emit_expr(sl->end.get(), out);
            else out << "-1";
            out << ")";
            break;
        }
        case NodeKind::MemberAccessExpr: {
            auto ma = static_cast<const AstMemberAccessExpr*>(e);
            emit_expr(ma->object.get(), out);
            out << "." << ma->member_name;
            break;
        }
        case NodeKind::ArrayLiteralExpr: {
            auto arr = static_cast<const AstArrayLiteralExpr*>(e);
            if (arr->is_fill()) {
                out << "mana::fill_array(";
                emit_expr(arr->fill_value.get(), out);
                out << ", ";
                emit_expr(arr->fill_count.get(), out);
                out << ")";
            } else {
                out << "{";
                for (size_t i = 0; i < arr->elements.size(); ++i) {
                    if (i > 0) out << ", ";
                    emit_expr(arr->elements[i].get(), out);
                }
                out << "}";
            }
            break;
        }
        case NodeKind::StructLiteralExpr: {
            auto sl = static_cast<const AstStructLiteralExpr*>(e);
            out << map_type(sl->type_name) << "{";
            for (size_t i = 0; i < sl->fields.size(); ++i) {
                if (i > 0) out << ", ";
                if (sl->is_named && !sl->fields[i].field_name.empty()) {
                    out << "." << sl->fields[i].field_name << " = ";
                }
                emit_expr(sl->fields[i].value.get(), out);
            }
            out << "}";
            break;
        }
        case NodeKind::ScopeAccessExpr: {
            auto sa = static_cast<const AstScopeAccessExpr*>(e);
            if (sa->member_name == "new") {
                out << "mana::" << sa->scope_name << "<>{}";
            } else {
                out << sa->scope_name << "::" << sa->member_name;
            }
            break;
        }
        case NodeKind::SelfExpr: { out << "self"; break; }
        case NodeKind::TupleExpr: {
            auto tup = static_cast<const AstTupleExpr*>(e);
            out << "std::make_tuple(";
            for (size_t i = 0; i < tup->elements.size(); ++i) {
                if (i > 0) out << ", ";
                emit_expr(tup->elements[i].get(), out);
            }
            out << ")";
            break;
        }
        case NodeKind::TupleIndexExpr: {
            auto ti = static_cast<const AstTupleIndexExpr*>(e);
            out << "std::get<" << ti->index << ">(";
            emit_expr(ti->tuple.get(), out);
            out << ")";
            break;
        }
        case NodeKind::RangeExpr: { out << "/* range */"; break; }
        case NodeKind::MatchExpr: {
            auto me = static_cast<const AstMatchExpr*>(e);
            int mcnt = match_counter++;
            out << "[&]() {\n";
            out << "        auto __match_value_" << mcnt << " = ";
            emit_expr(me->value.get(), out);
            out << ";\n";
            for (const auto& arm : me->arms) {
                bool is_wildcard = false;
                if (arm.patterns.size() == 1) {
                    if (auto id = dynamic_cast<const AstIdentifierExpr*>(arm.patterns[0].get())) {
                        if (id->name == "_") is_wildcard = true;
                    }
                }
                if (is_wildcard) {
                    out << "        return ";
                    emit_expr(arm.result.get(), out);
                    out << ";\n";
                } else {
                    // Check for enum pattern destructuring
                    if (auto enumPat = dynamic_cast<const AstEnumPattern*>(arm.patterns[0].get())) {
                        bool is_adt = adt_enums_.count(enumPat->enum_name) > 0;
                        if (is_adt) {
                            // ADT enum - use tag comparison and data extraction
                            out << "        if (__match_value_" << mcnt << ".tag == " << enumPat->enum_name << "Tag::" << enumPat->variant_name << ") {\n";
                            // Extract and bind data if pattern has bindings
                            if (!enumPat->bindings.empty()) {
                                out << "            auto __data_" << mcnt << " = std::get<" << enumPat->enum_name << "_" << enumPat->variant_name << ">(__match_value_" << mcnt << ".data);\n";
                                for (size_t j = 0; j < enumPat->bindings.size(); ++j) {
                                    if (enumPat->bindings[j] != "_") {
                                        out << "            auto " << enumPat->bindings[j] << " = __data_" << mcnt << "._" << j << ";\n";
                                    }
                                }
                            } else if (!enumPat->field_bindings.empty()) {
                                out << "            auto __data_" << mcnt << " = std::get<" << enumPat->enum_name << "_" << enumPat->variant_name << ">(__match_value_" << mcnt << ".data);\n";
                                for (const auto& fb : enumPat->field_bindings) {
                                    out << "            auto " << fb.second << " = __data_" << mcnt << "." << fb.first << ";\n";
                                }
                            }
                            if (arm.guard) {
                                out << "            if (";
                                emit_expr(arm.guard.get(), out);
                                out << ") ";
                            } else {
                                out << "            ";
                            }
                            out << "return ";
                            emit_expr(arm.result.get(), out);
                            out << ";\n";
                            out << "        }\n";
                        } else {
                            // Simple enum - use direct value comparison
                            out << "        if (__match_value_" << mcnt << " == " << enumPat->enum_name << "::" << enumPat->variant_name << ") ";
                            if (arm.guard) {
                                out << "&& (";
                                emit_expr(arm.guard.get(), out);
                                out << ") ";
                            }
                            out << "return ";
                            emit_expr(arm.result.get(), out);
                            out << ";\n";
                        }
                    } else {
                        out << "        if (";
                        if (arm.patterns.size() > 1) out << "(";
                        for (size_t i = 0; i < arm.patterns.size(); ++i) {
                            if (i > 0) out << " || ";
                            // Check if pattern is a range expression
                            if (auto range = dynamic_cast<const AstRangeExpr*>(arm.patterns[i].get())) {
                                out << "(__match_value_" << mcnt << " >= ";
                                emit_expr(range->start.get(), out);
                                out << " && __match_value_" << mcnt;
                                out << (range->inclusive ? " <= " : " < ");
                                emit_expr(range->end.get(), out);
                                out << ")";
                            } else if (auto scopeAccess = dynamic_cast<const AstScopeAccessExpr*>(arm.patterns[i].get())) {
                                // Check if it's an ADT enum or simple enum
                                if (adt_enums_.count(scopeAccess->scope_name)) {
                                    out << "__match_value_" << mcnt << ".tag == " << scopeAccess->scope_name << "Tag::" << scopeAccess->member_name;
                                } else {
                                    out << "__match_value_" << mcnt << " == " << scopeAccess->scope_name << "::" << scopeAccess->member_name;
                                }
                            } else {
                                out << "__match_value_" << mcnt << " == ";
                                emit_expr(arm.patterns[i].get(), out);
                            }
                        }
                        if (arm.patterns.size() > 1) out << ")";
                        if (arm.guard) {
                            out << " && (";
                            emit_expr(arm.guard.get(), out);
                            out << ")";
                        }
                        out << ") return ";
                        emit_expr(arm.result.get(), out);
                        out << ";\n";
                    }
                }
            }
            // If no wildcard/default case exists, throw exception for unmatched patterns
            if (!me->has_default) {
                out << "        throw std::runtime_error(\"non-exhaustive match\");\n";
            }
            out << "    }()";
            break;
        }
        case NodeKind::ClosureExpr: {
            auto cl = static_cast<const AstClosureExpr*>(e);
            emit_capture_list(cl, out); out << "(";
            for (size_t i = 0; i < cl->params.size(); ++i) {
                if (i > 0) out << ", ";
                if (!cl->params[i].type_name.empty()) {
                    out << map_type(cl->params[i].type_name) << " ";
                } else {
                    out << "auto ";
                }
                out << cl->params[i].name;
            }
            out << ")";
            // For mutable closures (move captures), add mutable keyword
            for (const auto& cap : cl->captures) {
                if (cap.mode == CaptureMode::ByMove) {
                    out << " mutable";
                    break;
                }
            }
            if (!cl->return_type.empty()) {
                out << " -> " << map_type(cl->return_type);
            }
            if (cl->has_block()) {
                out << " {\n";
                for (const auto& stmt : cl->body_block->statements) {
                    emit_stmt(stmt.get(), out, 2);
                }
                out << "    }";
            } else {
                out << " { return ";
                emit_expr(cl->body_expr.get(), out);
                out << "; }";
            }
            break;
        }
        case NodeKind::TryExpr: {
            auto te = static_cast<const AstTryExpr*>(e);
            auto it = try_expr_ids_.find(te);
            if (it != try_expr_ids_.end()) {
                out << "__try_" << it->second << ".__unwrap_ok()";
            } else {
                int tcnt = try_counter++;
                out << "[&]() {\n";
                out << "        auto __try_" << tcnt << " = ";
                emit_expr(te->operand.get(), out);
                out << ";\n";
                out << "        if (__try_" << tcnt << ".__is_err()) {\n";
                out << "            throw std::runtime_error(\"error propagation\");\n";
                out << "        }\n";
                out << "        return __try_" << tcnt << ".__unwrap_ok();\n";
                out << "    }()";
            }
            break;
        }
        case NodeKind::AwaitExpr: {
            auto ae = static_cast<const AstAwaitExpr*>(e);
            // Emit: operand.get() to get the future's result
            emit_expr(ae->operand.get(), out);
            out << ".get()";
            break;
        }
        case NodeKind::OptionalChainExpr: {
            auto oc = static_cast<const AstOptionalChainExpr*>(e);
            // Generate: [&]() { auto __opt = <obj>; if (__opt.is_none()) return None; return Option<T>(Some(...)); }()
            static int opt_counter = 0;
            int ocnt = opt_counter++;
            out << "[&]() {\n";
            out << "        auto __opt_" << ocnt << " = ";
            emit_expr(oc->object.get(), out);
            out << ";\n";
            out << "        if (__opt_" << ocnt << ".is_none()) return mana::make_none<decltype(__opt_" << ocnt << ".unwrap()." << oc->member_name;
            if (oc->is_method_call) {
                out << "(";
                for (size_t i = 0; i < oc->args.size(); ++i) {
                    if (i > 0) out << ", ";
                    emit_expr(dynamic_cast<const AstExpr*>(oc->args[i].get()), out);
                }
                out << ")";
            }
            out << ")>();\n";
            // Use explicit Option type to match make_none return type
            out << "        return mana::Option<decltype(__opt_" << ocnt << ".unwrap()." << oc->member_name;
            if (oc->is_method_call) {
                out << "(";
                for (size_t i = 0; i < oc->args.size(); ++i) {
                    if (i > 0) out << ", ";
                    emit_expr(dynamic_cast<const AstExpr*>(oc->args[i].get()), out);
                }
                out << ")";
            }
            out << ")>(mana::Some(__opt_" << ocnt << ".unwrap()." << oc->member_name;
            if (oc->is_method_call) {
                out << "(";
                for (size_t i = 0; i < oc->args.size(); ++i) {
                    if (i > 0) out << ", ";
                    emit_expr(dynamic_cast<const AstExpr*>(oc->args[i].get()), out);
                }
                out << ")";
            }
            out << "));\n";
            out << "    }()";
            break;
        }
        case NodeKind::NullCoalesceExpr: {
            auto nc = static_cast<const AstNullCoalesceExpr*>(e);
            // Generate: [&]() -> std::decay_t<decltype(opt.unwrap())> { auto __nc = opt; if (__nc.is_some()) return __nc.unwrap(); return default; }()
            static int nc_counter = 0;
            int ncnt = nc_counter++;
            out << "[&]() -> std::decay_t<decltype(";
            emit_expr(nc->option_expr.get(), out);
            out << ".unwrap())> {\n";
            out << "        auto __nc_" << ncnt << " = ";
            emit_expr(nc->option_expr.get(), out);
            out << ";\n";
            out << "        if (__nc_" << ncnt << ".is_some()) return __nc_" << ncnt << ".unwrap();\n";
            out << "        return ";
            emit_expr(nc->default_expr.get(), out);
            out << ";\n";
            out << "    }()";
            break;
        }
        case NodeKind::NoneExpr: { out << "mana::None"; break; }
        case NodeKind::OptionPattern: {
            auto op = static_cast<const AstOptionPattern*>(e);
            if (op->pattern_kind == "Some" || op->pattern_kind == "some") out << "mana::Some(" << op->binding << ")";
            else if (op->pattern_kind == "None" || op->pattern_kind == "none") out << "mana::None";
            else if (op->pattern_kind == "Ok" || op->pattern_kind == "ok") out << "mana::Ok(" << op->binding << ")";
            else if (op->pattern_kind == "Err" || op->pattern_kind == "err") out << "mana::Err(" << op->binding << ")";
            break;
        }
        case NodeKind::CastExpr: {
            auto ce = static_cast<const AstCastExpr*>(e);
            out << "static_cast<" << map_type(ce->target_type) << ">(";
            emit_expr(ce->operand.get(), out);
            out << ")";
            break;
        }
        case NodeKind::IfExpr: {
            auto ie = static_cast<const AstIfExpr*>(e);
            out << "(";
            emit_expr(ie->condition.get(), out);
            out << " ? ";
            emit_expr(ie->then_expr.get(), out);
            out << " : ";
            emit_expr(ie->else_expr.get(), out);
            out << ")";
            break;
        }
        case NodeKind::OrExpr: {
            // expr or return/break/{ block }
            // Desugar to: [&]() { auto __or_N = expr; if (__or_N.is_ok()) return __or_N.unwrap(); fallback; }()
            static int or_counter = 0;
            int ocnt = or_counter++;
            auto oe = static_cast<const AstOrExpr*>(e);
            out << "[&]() {\n";
            out << "        auto __or_" << ocnt << " = ";
            emit_expr(oe->lhs.get(), out);
            out << ";\n";
            out << "        if (__or_" << ocnt << ".is_ok()) return __or_" << ocnt << ".unwrap();\n";
            // Emit fallback
            if (oe->has_block()) {
                for (const auto& stmt : oe->fallback_block->statements) {
                    emit_stmt(stmt.get(), out, 2);
                }
            } else if (oe->fallback_stmt) {
                emit_stmt(oe->fallback_stmt.get(), out, 2);
            }
            out << "    }()";
            break;
        }
        default: out << "/* unknown expr */"; break;
    }
}

void CppEmitter::extract_try_exprs(const AstExpr* e, std::ostream& out, int ind) {
    if (!e) return;
    
    if (auto te = dynamic_cast<const AstTryExpr*>(e)) {
        int tcnt = try_counter++;
        try_expr_ids_[te] = tcnt;
        indent(out, ind);
        out << "auto __try_" << tcnt << " = ";
        emit_expr(te->operand.get(), out);
        out << ";\n";
        indent(out, ind);
        out << "if (__try_" << tcnt << ".__is_err()) return mana::Err(__try_" << tcnt << ".__unwrap_err());\n";
        return;
    }
    
    switch (e->kind) {
        case NodeKind::BinaryExpr: {
            auto bin = static_cast<const AstBinaryExpr*>(e);
            extract_try_exprs(static_cast<const AstExpr*>(bin->left.get()), out, ind);
            extract_try_exprs(static_cast<const AstExpr*>(bin->right.get()), out, ind);
            break;
        }
        case NodeKind::UnaryExpr: {
            auto un = static_cast<const AstUnaryExpr*>(e);
            extract_try_exprs(static_cast<const AstExpr*>(un->right.get()), out, ind);
            break;
        }
        case NodeKind::CallExpr: {
            auto call = static_cast<const AstCallExpr*>(e);
            for (const auto& arg : call->args) {
                extract_try_exprs(static_cast<const AstExpr*>(arg.get()), out, ind);
            }
            break;
        }
        case NodeKind::MethodCallExpr: {
            auto mc = static_cast<const AstMethodCallExpr*>(e);
            extract_try_exprs(mc->object.get(), out, ind);
            for (const auto& arg : mc->args) {
                extract_try_exprs(static_cast<const AstExpr*>(arg.get()), out, ind);
            }
            break;
        }
        case NodeKind::IndexExpr: {
            auto idx = static_cast<const AstIndexExpr*>(e);
            extract_try_exprs(idx->base.get(), out, ind);
            extract_try_exprs(idx->index.get(), out, ind);
            break;
        }
        case NodeKind::CastExpr: {
            auto ce = static_cast<const AstCastExpr*>(e);
            extract_try_exprs(ce->operand.get(), out, ind);
            break;
        }
        default:
            break;
    }
}

void CppEmitter::emit_stmt(const AstStmt* s, std::ostream& out, int ind) {
    if (!s) return;
    switch (s->kind) {
        case NodeKind::BlockStmt: {
            auto blk = static_cast<const AstBlockStmt*>(s);
            out << "{\n";
            for (const auto& stmt : blk->statements) {
                emit_stmt(stmt.get(), out, ind + 1);
            }
            indent(out, ind);
            out << "}";
            break;
        }
        case NodeKind::VarDeclStmt: {
            auto vd = static_cast<const AstVarDeclStmt*>(s);
            if (auto ds = dynamic_cast<const AstDestructureStmt*>(s)) {
                indent(out, ind);
                int dcnt = destructure_counter++;
                out << "auto __ds_" << dcnt << " = ";
                emit_expr(static_cast<const AstExpr*>(ds->init_expr.get()), out);
                out << ";\n";
                for (size_t i = 0; i < ds->bindings.size(); ++i) {
                    indent(out, ind);
                    out << "auto " << ds->bindings[i].name << " = ";
                    if (ds->is_tuple) {
                        out << "std::get<" << i << ">(__ds_" << dcnt << ")";
                    } else if (ds->is_struct) {
                        out << "__ds_" << dcnt << "." << ds->bindings[i].field_name;
                    } else {
                        out << "__ds_" << dcnt << "[" << i << "]";
                    }
                    out << ";\n";
                }
            } else {
                if (vd->init_expr) {
                    extract_try_exprs(static_cast<const AstExpr*>(vd->init_expr.get()), out, ind);
                }
                indent(out, ind);
                if (!vd->type_name.empty()) {
                    out << map_type(vd->type_name) << " ";
                } else {
                    out << "auto ";
                }
                out << vd->name;
                if (vd->init_expr) {
                    out << " = ";
                    emit_expr(static_cast<const AstExpr*>(vd->init_expr.get()), out);
                }
                out << ";\n";
            }
            break;
        }
        case NodeKind::AssignStmt: {
            auto as = static_cast<const AstAssignStmt*>(s);
            extract_try_exprs(static_cast<const AstExpr*>(as->value.get()), out, ind);
            indent(out, ind);
            if (as->is_complex_target()) {
                emit_expr(static_cast<const AstExpr*>(as->target_expr.get()), out);
            } else {
                out << as->target_name;
            }
            out << " " << as->op << " ";
            emit_expr(static_cast<const AstExpr*>(as->value.get()), out);
            out << ";\n";
            break;
        }
        case NodeKind::IfStmt: {
            auto ifs = static_cast<const AstIfStmt*>(s);
            indent(out, ind);
            if (ifs->is_if_let) {
                std::string check_method, unwrap_method;
                if (ifs->pattern_kind == "Some" || ifs->pattern_kind == "some") { check_method = "is_some()"; unwrap_method = "unwrap()"; }
                else if (ifs->pattern_kind == "None" || ifs->pattern_kind == "none") { check_method = "is_none()"; unwrap_method = ""; }
                else if (ifs->pattern_kind == "Ok" || ifs->pattern_kind == "ok") { check_method = "is_ok()"; unwrap_method = "unwrap()"; }
                else if (ifs->pattern_kind == "Err" || ifs->pattern_kind == "err") { check_method = "is_err()"; unwrap_method = "unwrap_err()"; }
                out << "if (";
                emit_expr(static_cast<const AstExpr*>(ifs->pattern_expr.get()), out);
                out << "." << check_method << ") {\n";
                if (!ifs->pattern_var.empty() && !unwrap_method.empty()) {
                    indent(out, ind + 1);
                    out << "auto " << ifs->pattern_var << " = ";
                    emit_expr(static_cast<const AstExpr*>(ifs->pattern_expr.get()), out);
                    out << "." << unwrap_method << ";\n";
                }
                auto blk = static_cast<const AstBlockStmt*>(ifs->then_block.get());
                for (const auto& stmt : blk->statements) {
                    emit_stmt(stmt.get(), out, ind + 1);
                }
                indent(out, ind);
                out << "}";
            } else {
                out << "if (";
                emit_expr(static_cast<const AstExpr*>(ifs->condition.get()), out);
                out << ") ";
                emit_stmt(ifs->then_block.get(), out, ind);
            }
            if (ifs->else_block) {
                out << " else ";
                emit_stmt(ifs->else_block.get(), out, ind);
            }
            out << "\n";
            break;
        }
        case NodeKind::WhileStmt: {
            auto ws = static_cast<const AstWhileStmt*>(s);
            indent(out, ind);
            if (ws->is_while_let) {
                int wcnt = while_let_counter++;
                std::string check_method, unwrap_method;
                if (ws->pattern_kind == "Some" || ws->pattern_kind == "some") { check_method = "is_some()"; unwrap_method = "unwrap()"; }
                else if (ws->pattern_kind == "Ok" || ws->pattern_kind == "ok") { check_method = "is_ok()"; unwrap_method = "unwrap()"; }
                else if (ws->pattern_kind == "Err" || ws->pattern_kind == "err") { check_method = "is_err()"; unwrap_method = "unwrap_err()"; }
                out << "while (true) {\n";
                indent(out, ind + 1);
                out << "auto __wl_" << wcnt << " = ";
                emit_expr(static_cast<const AstExpr*>(ws->pattern_expr.get()), out);
                out << ";\n";
                indent(out, ind + 1);
                out << "if (!__wl_" << wcnt << "." << check_method << ") break;\n";
                if (!ws->pattern_var.empty()) {
                    indent(out, ind + 1);
                    out << "auto " << ws->pattern_var << " = __wl_" << wcnt << "." << unwrap_method << ";\n";
                }
                auto blk = static_cast<const AstBlockStmt*>(ws->body.get());
                for (const auto& stmt : blk->statements) {
                    emit_stmt(stmt.get(), out, ind + 1);
                }
                indent(out, ind);
                out << "}\n";
            } else {
                out << "while (";
                emit_expr(static_cast<const AstExpr*>(ws->condition.get()), out);
                out << ") ";
                emit_stmt(ws->body.get(), out, ind);
                out << "\n";
            }
            break;
        }
        case NodeKind::ForInStmt: {
            auto fin = static_cast<const AstForInStmt*>(s);
            indent(out, ind);
            if (auto rng = dynamic_cast<const AstRangeExpr*>(fin->iterable.get())) {
                out << "for (int32_t " << fin->var_name << " = ";
                emit_expr(rng->start.get(), out);
                out << "; " << fin->var_name;
                if (rng->inclusive) out << " <= ";
                else out << " < ";
                emit_expr(rng->end.get(), out);
                out << "; ++" << fin->var_name << ") ";
            } else if (fin->is_destructure) {
                out << "for (auto& [";
                for (size_t i = 0; i < fin->var_names.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << fin->var_names[i];
                }
                out << "] : ";
                emit_expr(static_cast<const AstExpr*>(fin->iterable.get()), out);
                out << ") ";
            } else {
                out << "for (auto " << fin->var_name << " : ";
                emit_expr(static_cast<const AstExpr*>(fin->iterable.get()), out);
                out << ") ";
            }
            emit_stmt(fin->body.get(), out, ind);
            out << "\n";
            break;
        }
        case NodeKind::ForStmt: {
            auto fs = static_cast<const AstForStmt*>(s);
            indent(out, ind);
            out << "for (";
            if (fs->init) {
                if (auto vd = dynamic_cast<const AstVarDeclStmt*>(fs->init.get())) {
                    if (!vd->type_name.empty()) out << map_type(vd->type_name) << " ";
                    else out << "auto ";
                    out << vd->name;
                    if (vd->init_expr) {
                        out << " = ";
                        emit_expr(static_cast<const AstExpr*>(vd->init_expr.get()), out);
                    }
                }
            }
            out << "; ";
            if (fs->condition) emit_expr(static_cast<const AstExpr*>(fs->condition.get()), out);
            out << "; ";
            if (fs->increment) {
                if (auto as = dynamic_cast<const AstAssignStmt*>(fs->increment.get())) {
                    if (as->is_complex_target()) emit_expr(static_cast<const AstExpr*>(as->target_expr.get()), out);
                    else out << as->target_name;
                    out << " " << as->op << " ";
                    emit_expr(static_cast<const AstExpr*>(as->value.get()), out);
                }
            }
            out << ") ";
            emit_stmt(fs->body.get(), out, ind);
            out << "\n";
            break;
        }
        case NodeKind::LoopStmt: {
            auto ls = static_cast<const AstLoopStmt*>(s);
            indent(out, ind);
            out << "while (true) ";
            emit_stmt(ls->body.get(), out, ind);
            out << "\n";
            break;
        }
        case NodeKind::BreakStmt: {
            auto bs = static_cast<const AstBreakStmt*>(s);
            indent(out, ind);
            if (bs->value) {
                out << "__loop_result = ";
                emit_expr(bs->value.get(), out);
                out << "; ";
            }
            out << "break;\n";
            break;
        }
        case NodeKind::ContinueStmt: {
            indent(out, ind);
            out << "continue;\n";
            break;
        }
        case NodeKind::ReturnStmt: {
            auto rs = static_cast<const AstReturnStmt*>(s);
            indent(out, ind);
            out << "return";
            if (rs->value) {
                out << " ";
                emit_expr(static_cast<const AstExpr*>(rs->value.get()), out);
            }
            out << ";\n";
            break;
        }
        case NodeKind::DeferStmt: {
            indent(out, ind);
            out << "// defer statement (not fully implemented)\n";
            break;
        }
        case NodeKind::ScopeStmt: {
            auto ss = static_cast<const AstScopeStmt*>(s);
            indent(out, ind);
            out << "{\n";
            if (!ss->name.empty() && ss->init_expr) {
                indent(out, ind + 1);
                out << "auto " << ss->name << " = ";
                emit_expr(static_cast<const AstExpr*>(ss->init_expr.get()), out);
                out << ";\n";
            }
            if (ss->body) {
                auto blk = static_cast<const AstBlockStmt*>(ss->body.get());
                for (const auto& stmt : blk->statements) {
                    emit_stmt(stmt.get(), out, ind + 1);
                }
            }
            indent(out, ind);
            out << "}\n";
            break;
        }
        case NodeKind::ExprStmt: {
            auto es = static_cast<const AstExprStmt*>(s);
            extract_try_exprs(static_cast<const AstExpr*>(es->expr.get()), out, ind);
            indent(out, ind);
            if (auto call = dynamic_cast<const AstCallExpr*>(es->expr.get())) {
                if (call->func_name == "print" && call->args.size() > 1) {
                    out << "([&]{ ";
                    for (size_t i = 0; i < call->args.size(); ++i) {
                        out << "std::cout << ";
                        emit_expr(static_cast<const AstExpr*>(call->args[i].get()), out);
                        out << "; ";
                    }
                    out << "}());\n";
                    break;
                } else if (call->func_name == "println" && call->args.size() > 1) {
                    out << "([&]{ ";
                    for (size_t i = 0; i < call->args.size(); ++i) {
                        out << "std::cout << ";
                        emit_expr(static_cast<const AstExpr*>(call->args[i].get()), out);
                        out << "; ";
                    }
                    out << "std::cout << std::endl; }());\n";
                    break;
                }
            }
            emit_expr(static_cast<const AstExpr*>(es->expr.get()), out);
            out << ";\n";
            break;
        }
        default:
            indent(out, ind);
            out << "/* unknown stmt */\n";
            break;
    }
}

void CppEmitter::emit(const AstModule* m, std::ostream& out, bool test_mode) {
    test_mode_ = test_mode;
    match_counter = 0;
    try_counter = 0;
    destructure_counter = 0;
    while_let_counter = 0;

    // Pre-pass: register all impl methods for method call resolution
    impl_methods_.clear();  // Clear for fresh compile
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::ImplDecl) {
            auto impl = static_cast<const AstImplDecl*>(decl.get());
            for (const auto& method : impl->methods) {
                impl_methods_.insert(impl->type_name + "_" + method->name);
            }
        }
    }

    out << "// Generated by mana-compiler\n";
    out << "#include <cstdint>\n";
    out << "#include <string>\n";
    out << "#include <array>\n";
    out << "#include <vector>\n";
    out << "#include <tuple>\n";
    out << "#include <cmath>\n";
    out << "#include <type_traits>\n";
    out << "#include <variant>\n";
    out << "#include <future>\n";
    out << "#include \"mana_runtime.h\"\n";

    // Emit comments for use declarations
    for (const auto& decl : m->decls) {
        if (decl->kind == mana::frontend::NodeKind::UseDecl) {
            auto use = static_cast<mana::frontend::AstUseDecl*>(decl.get());
            std::string path = use->module_path;
            if (path.rfind("std::", 0) == 0) {
                out << "// use " << path;
                if (use->is_glob) out << "::*";
                if (!use->alias.empty()) out << " as " << use->alias;
                if (!use->imported_names.empty()) {
                    out << "::{" << use->imported_names[0];
                    for (size_t i = 1; i < use->imported_names.size(); ++i) {
                        out << ", " << use->imported_names[i];
                    }
                    out << "}";
                }
                out << ";\n";
            } else {
                std::string include_path = path;
                size_t pos;
                while ((pos = include_path.find("::")) != std::string::npos) {
                    include_path.replace(pos, 2, "/");
                }
                out << "#include \"" << include_path << ".h\"\n";
            }
        }
    }
    out << "\n";

    // Emit type aliases first (before structs that might use them)
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::TypeAliasDecl) {
            auto alias = static_cast<const AstTypeAliasDecl*>(decl.get());
            out << "using " << alias->alias_name << " = " << map_type(alias->target_type) << ";\n";
        }
    }
    out << "\n";

    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::StructDecl) {
            auto sd = static_cast<const AstStructDecl*>(decl.get());
            if (sd->is_generic()) {
                out << "template<";
                for (size_t i = 0; i < sd->type_params.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << "typename " << sd->type_params[i];
                }
                out << ">\n";
            }
            out << "struct " << sd->name << " {\n";
            for (const auto& f : sd->fields) {
                out << "    " << map_type(f.type_name) << " " << f.name << ";\n";
            }
            out << "};\n\n";
        } else if (decl->kind == NodeKind::EnumDecl) {
            auto ed = static_cast<const AstEnumDecl*>(decl.get());
            if (ed->has_data_variants()) {
                adt_enums_.insert(ed->name);
                // Algebraic data type - generate tagged union
                for (const auto& v : ed->variants) {
                    if (v.is_tuple_variant()) {
                        out << "struct " << ed->name << "_" << v.name << " {\n";
                        for (size_t j = 0; j < v.tuple_types.size(); ++j) {
                            out << "    " << map_type(v.tuple_types[j]) << " _" << j << ";\n";
                        }
                        out << "};\n\n";
                    } else if (v.is_struct_variant()) {
                        out << "struct " << ed->name << "_" << v.name << " {\n";
                        for (const auto& f : v.struct_fields) {
                            out << "    " << map_type(f.type_name) << " " << f.name << ";\n";
                        }
                        out << "};\n\n";
                    }
                }
                out << "enum class " << ed->name << "Tag {\n";
                for (size_t i = 0; i < ed->variants.size(); ++i) {
                    out << "    " << ed->variants[i].name;
                    if (i + 1 < ed->variants.size()) out << ",";
                    out << "\n";
                }
                out << "};\n\n";
                out << "struct " << ed->name << " {\n";
                out << "    " << ed->name << "Tag tag;\n";
                out << "    std::variant<std::monostate";
                for (const auto& v : ed->variants) {
                    if (v.has_data()) {
                        out << ", " << ed->name << "_" << v.name;
                    }
                }
                out << "> data;\n\n";
                for (const auto& v : ed->variants) {
                    out << "    static " << ed->name << " " << v.name << "(";
                    if (v.is_tuple_variant()) {
                        for (size_t j = 0; j < v.tuple_types.size(); ++j) {
                            if (j > 0) out << ", ";
                            out << map_type(v.tuple_types[j]) << " v" << j;
                        }
                        out << ") {\n";
                        out << "        return " << ed->name << "{" << ed->name << "Tag::" << v.name << ", " << ed->name << "_" << v.name << "{";
                        for (size_t j = 0; j < v.tuple_types.size(); ++j) {
                            if (j > 0) out << ", ";
                            out << "v" << j;
                        }
                        out << "}};\n";
                    } else if (v.is_struct_variant()) {
                        for (size_t j = 0; j < v.struct_fields.size(); ++j) {
                            if (j > 0) out << ", ";
                            out << map_type(v.struct_fields[j].type_name) << " " << v.struct_fields[j].name;
                        }
                        out << ") {\n";
                        out << "        return " << ed->name << "{" << ed->name << "Tag::" << v.name << ", " << ed->name << "_" << v.name << "{";
                        for (size_t j = 0; j < v.struct_fields.size(); ++j) {
                            if (j > 0) out << ", ";
                            out << v.struct_fields[j].name;
                        }
                        out << "}};\n";
                    } else {
                        out << ") {\n";
                        out << "        return " << ed->name << "{" << ed->name << "Tag::" << v.name << ", std::monostate{}};\n";
                    }
                    out << "    }\n";
                }
                out << "};\n\n";
            } else {
                out << "enum class " << ed->name << " {\n";
                for (size_t i = 0; i < ed->variants.size(); ++i) {
                    out << "    " << ed->variants[i].name;
                    if (ed->variants[i].has_value) out << " = " << ed->variants[i].value;
                    if (i + 1 < ed->variants.size()) out << ",";
                    out << "\n";
                }
                out << "};\n\n";
            }
        }
    }

    // Emit trait declarations as abstract C++ classes (for dynamic dispatch)
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::TraitDecl) {
            auto td = static_cast<const AstTraitDecl*>(decl.get());
            out << "// Trait interface for dynamic dispatch\n";
            out << "class I" << td->name << " {\n";
            out << "public:\n";
            out << "    virtual ~I" << td->name << "() = default;\n";
            for (const auto& method : td->methods) {
                out << "    virtual ";
                if (method.return_type.empty()) out << "void ";
                else out << map_type(method.return_type) << " ";
                out << method.name << "(";
                for (size_t i = 0; i < method.params.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << map_type(method.params[i].type_name) << " " << method.params[i].name;
                }
                out << ")" << (method.has_default() ? "" : " = 0") << ";\n";
            }
            out << "};\n\n";
        }
    }

    // Emit trait implementations as wrapper classes
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::ImplDecl) {
            auto impl = static_cast<const AstImplDecl*>(decl.get());
            if (!impl->trait_name.empty()) {
                // This is a trait impl - generate a wrapper class
                out << "// Wrapper for " << impl->type_name << " implementing I" << impl->trait_name << "\n";
                out << "class " << impl->type_name << "_" << impl->trait_name << "_Impl : public I" << impl->trait_name << " {\n";
                out << "    " << impl->type_name << "& inner_;\n";
                out << "public:\n";
                out << "    explicit " << impl->type_name << "_" << impl->trait_name << "_Impl(" << impl->type_name << "& obj) : inner_(obj) {}\n";
                for (const auto& method : impl->methods) {
                    out << "    ";
                    if (method->return_type.empty()) out << "void ";
                    else out << map_type(method->return_type) << " ";
                    out << method->name << "(";
                    for (size_t i = 0; i < method->params.size(); ++i) {
                        if (i > 0) out << ", ";
                        out << map_type(method->params[i].type_name) << " " << method->params[i].name;
                    }
                    out << ") override {\n";
                    out << "        ";
                    if (!method->return_type.empty()) out << "return ";
                    out << impl->type_name << "_" << method->name << "(inner_";
                    for (const auto& param : method->params) {
                        out << ", " << param.name;
                    }
                    out << ");\n";
                    out << "    }\n";
                }
                out << "};\n\n";
                
                // Generate helper function to create trait object
                out << "std::unique_ptr<I" << impl->trait_name << "> make_" << impl->trait_name << "(" << impl->type_name << "& obj) {\n";
                out << "    return std::make_unique<" << impl->type_name << "_" << impl->trait_name << "_Impl>(obj);\n";
                out << "}\n\n";
            }
        }
    }

    // Emit forward declarations for all functions (enables forward references)
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::FunctionDecl) {
            auto fd = static_cast<const AstFuncDecl*>(decl.get());
            // Skip main - it doesn't need forward declaration
            if (fd->name == "main") continue;
            if (fd->is_generic()) {
                out << "template<";
                for (size_t i = 0; i < fd->type_params.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << "typename " << fd->type_params[i];
                }
                out << ">\n";
            }
            if (fd->is_async) {
                if (fd->return_type.empty()) out << "std::future<void> ";
                else out << "std::future<" << map_type(fd->return_type) << "> ";
            } else {
                if (fd->return_type.empty()) out << "void ";
                else out << map_type(fd->return_type) << " ";
            }
            if (fd->is_method()) out << fd->receiver_type << "_" << fd->name;
            else out << fd->name;
            out << "(";
            if (fd->is_method()) {
                out << fd->receiver_type << "& self";
                if (!fd->params.empty()) out << ", ";
            }
            for (size_t i = 0; i < fd->params.size(); ++i) {
                if (i > 0) out << ", ";
                out << map_type(fd->params[i].type_name) << " " << fd->params[i].name;
            }
            out << ");\n";
        }
    }
    out << "\n";

    // Emit function implementations
    for (const auto& decl : m->decls) {
        if (decl->kind == NodeKind::FunctionDecl) {
            auto fd = static_cast<const AstFuncDecl*>(decl.get());
            if (fd->is_generic()) {
                out << "template<";
                for (size_t i = 0; i < fd->type_params.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << "typename " << fd->type_params[i];
                }
                out << ">\n";
            }
            // Handle async functions - return std::future<T>
            if (fd->is_async) {
                if (fd->return_type.empty()) out << "std::future<void> ";
                else out << "std::future<" << map_type(fd->return_type) << "> ";
            } else {
                if (fd->return_type.empty()) out << "void ";
                else out << map_type(fd->return_type) << " ";
            }
            if (fd->is_method()) out << fd->receiver_type << "_" << fd->name;
            else out << fd->name;
            out << "(";
            if (fd->is_method()) {
                out << fd->receiver_type << "& self";
                if (!fd->params.empty()) out << ", ";
            }
            for (size_t i = 0; i < fd->params.size(); ++i) {
                if (i > 0) out << ", ";
                out << map_type(fd->params[i].type_name) << " " << fd->params[i].name;
                if (fd->params[i].has_default()) {
                    out << " = ";
                    emit_expr(fd->params[i].default_value.get(), out);
                }
            }
            out << ") {\n";
            if (fd->is_async) {
                // Wrap body in std::async
                out << "    return std::async(std::launch::async, [&]() {\n";
                if (fd->body) {
                    for (const auto& stmt : fd->body->statements) {
                        emit_stmt(stmt.get(), out, 2);
                    }
                }
                out << "    });\n";
            } else {
                if (fd->body) {
                    for (const auto& stmt : fd->body->statements) {
                        emit_stmt(stmt.get(), out, 1);
                    }
                }
                // Insert implicit return 0 for main if no return statement at end
                if (fd->name == "main" && !fd->is_method()) {
                    bool has_return_at_end = false;
                    if (fd->body && !fd->body->statements.empty()) {
                        auto* last = fd->body->statements.back().get();
                        has_return_at_end = (last->kind == NodeKind::ReturnStmt);
                    }
                    if (!has_return_at_end) {
                        indent(out, 1);
                        out << "return 0;\n";
                    }
                }
            }
            out << "}\n\n";
        } else if (decl->kind == NodeKind::ImplDecl) {
            auto impl = static_cast<const AstImplDecl*>(decl.get());
            for (const auto& method : impl->methods) {
                if (method->return_type.empty()) out << "void ";
                else out << map_type(method->return_type) << " ";
                out << impl->type_name << "_" << method->name << "(";
                // Only add self parameter for instance methods (not static)
                bool first_param = true;
                if (method->has_self) {
                    out << impl->type_name << "& self";
                    first_param = false;
                }
                for (size_t i = 0; i < method->params.size(); ++i) {
                    if (!first_param) out << ", ";
                    first_param = false;
                    out << map_type(method->params[i].type_name) << " " << method->params[i].name;
                    if (method->params[i].has_default()) {
                        out << " = ";
                        emit_expr(method->params[i].default_value.get(), out);
                    }
                }
                out << ") {\n";
                if (method->body) {
                    for (const auto& stmt : method->body->statements) {
                        emit_stmt(stmt.get(), out, 1);
                    }
                }
                out << "}\n\n";
            }
        } else if (decl->kind == NodeKind::GlobalVarDecl) {
            auto gv = static_cast<const AstGlobalVarDecl*>(decl.get());
            if (gv->var) {
                if (gv->var->is_mutable) out << map_type(gv->var->type_name) << " " << gv->var->name;
                else out << "const " << map_type(gv->var->type_name) << " " << gv->var->name;
                if (gv->var->init_expr) {
                    out << " = ";
                    emit_expr(static_cast<const AstExpr*>(gv->var->init_expr.get()), out);
                }
                out << ";\n";
            }
        }
    }
}

} // namespace mana::backend
