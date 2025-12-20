#include "Semantic.h"
#include "AstExpressions.h"
#include "AstStatements.h"
#include "AstDeclarations.h"
#include <unordered_set>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace mana::frontend {

    // Check if a trait name is a built-in operator trait
    static bool is_operator_trait(const std::string& name) {
        static const std::unordered_set<std::string> operator_traits = {
            "Add", "Sub", "Mul", "Div", "Rem",
            "Eq", "Ne", "Lt", "Gt", "Le", "Ge",
            "Neg", "Not",
            "BitAnd", "BitOr", "BitXor", "Shl", "Shr"
        };
        return operator_traits.count(name) > 0;
    }

    SemanticAnalyzer::SemanticAnalyzer(DiagnosticEngine& diag)
        : diag_(diag) {
    }

    void SemanticAnalyzer::register_builtins() {
        // Register built-in functions
        builtin_functions_["print"] = true;
        builtin_functions_["println"] = true;
        builtin_functions_["Ok"] = true;
        builtin_functions_["Err"] = true;
        builtin_functions_["Some"] = true;

        // String functions
        builtin_functions_["len"] = true;
        builtin_functions_["is_empty"] = true;
        builtin_functions_["to_string"] = true;
        builtin_functions_["starts_with"] = true;
        builtin_functions_["ends_with"] = true;
        builtin_functions_["contains"] = true;
        builtin_functions_["trim"] = true;
        builtin_functions_["substr"] = true;

        // Math functions
        builtin_functions_["abs"] = true;
        builtin_functions_["min"] = true;
        builtin_functions_["max"] = true;
        builtin_functions_["clamp"] = true;

        // I/O functions
        builtin_functions_["read_line"] = true;
        builtin_functions_["parse_int"] = true;
        builtin_functions_["parse_float"] = true;

        // Array/slice functions
        builtin_functions_["first"] = true;
        builtin_functions_["last"] = true;
        builtin_functions_["concat"] = true;
        builtin_functions_["flatten"] = true;
        builtin_functions_["zip"] = true;
        builtin_functions_["unzip"] = true;
        builtin_functions_["repeat"] = true;

        // File I/O functions
        builtin_functions_["read_file"] = true;
        builtin_functions_["write_file"] = true;
        builtin_functions_["append_file"] = true;
        builtin_functions_["file_exists"] = true;
        builtin_functions_["delete_file"] = true;
        builtin_functions_["read_lines"] = true;

        // Utility
        builtin_functions_["assert_true"] = true;

        // Add them to global scope as void functions
        declare("print", { "print", Type::void_(), false });
        declare("println", { "println", Type::void_(), false });

        // Error handling builtins - return unknown (type depends on context)
        declare("Ok", { "Ok", Type::unknown(), false });
        declare("Err", { "Err", Type::unknown(), false });
        declare("Some", { "Some", Type::unknown(), false });

        // String functions
        declare("len", { "len", Type::i32(), false });           // returns size
        declare("is_empty", { "is_empty", Type::boolean(), false });
        declare("to_string", { "to_string", Type::string(), false });
        declare("starts_with", { "starts_with", Type::boolean(), false });
        declare("ends_with", { "ends_with", Type::boolean(), false });
        declare("contains", { "contains", Type::boolean(), false });
        declare("trim", { "trim", Type::string(), false });
        declare("substr", { "substr", Type::string(), false });

        // Math functions - return i32 by default
        declare("abs", { "abs", Type::i32(), false });
        declare("min", { "min", Type::i32(), false });
        declare("max", { "max", Type::i32(), false });
        declare("clamp", { "clamp", Type::i32(), false });

        // Additional math functions - return f64
        declare("sqrt", { "sqrt", Type::f32(), false });
        declare("sin", { "sin", Type::f32(), false });
        declare("cos", { "cos", Type::f32(), false });
        declare("tan", { "tan", Type::f32(), false });
        declare("asin", { "asin", Type::f32(), false });
        declare("acos", { "acos", Type::f32(), false });
        declare("atan", { "atan", Type::f32(), false });
        declare("atan2", { "atan2", Type::f32(), false });
        declare("floor", { "floor", Type::f32(), false });
        declare("ceil", { "ceil", Type::f32(), false });
        declare("round", { "round", Type::f32(), false });
        declare("trunc", { "trunc", Type::f32(), false });
        declare("log", { "log", Type::f32(), false });
        declare("log10", { "log10", Type::f32(), false });
        declare("log2", { "log2", Type::f32(), false });
        declare("exp", { "exp", Type::f32(), false });
        declare("pow", { "pow", Type::f32(), false });

        // I/O functions
        declare("read_line", { "read_line", Type::string(), false });
        declare("parse_int", { "parse_int", Type::unknown(), false });  // returns Option<i32>
        declare("parse_float", { "parse_float", Type::unknown(), false }); // returns Option<f32>

        // Array/slice functions
        declare("first", { "first", Type::unknown(), false });
        declare("last", { "last", Type::unknown(), false });
        declare("concat", { "concat", Type::unknown(), false });
        declare("flatten", { "flatten", Type::unknown(), false });
        declare("zip", { "zip", Type::unknown(), false });
        declare("unzip", { "unzip", Type::unknown(), false });
        declare("repeat", { "repeat", Type::unknown(), false });

        // File I/O functions
        declare("read_file", { "read_file", Type::unknown(), false });    // returns Result<string, string>
        declare("write_file", { "write_file", Type::unknown(), false });  // returns Result<void, string>
        declare("append_file", { "append_file", Type::unknown(), false }); // returns Result<void, string>
        declare("file_exists", { "file_exists", Type::boolean(), false });
        declare("delete_file", { "delete_file", Type::unknown(), false }); // returns Result<void, string>
        declare("read_lines", { "read_lines", Type::unknown(), false });  // returns Result<Vec<string>, string>

        // Utility & assert functions
        declare("assert_true", { "assert_true", Type::void_(), false });
        declare("assert_false", { "assert_false", Type::void_(), false });
        declare("assert", { "assert", Type::void_(), false });
        declare("assert_eq", { "assert_eq", Type::void_(), false });
        declare("assert_ne", { "assert_ne", Type::void_(), false });
        declare("assert_msg", { "assert_msg", Type::void_(), false });
        declare("assert_some", { "assert_some", Type::void_(), false });
        declare("assert_none", { "assert_none", Type::void_(), false });
        declare("assert_ok", { "assert_ok", Type::void_(), false });
        declare("assert_err", { "assert_err", Type::void_(), false });
        declare("assert_contains", { "assert_contains", Type::void_(), false });
        declare("assert_empty", { "assert_empty", Type::void_(), false });
        declare("assert_len", { "assert_len", Type::void_(), false });
        declare("assert_str_eq", { "assert_str_eq", Type::void_(), false });
        declare("assert_gt", { "assert_gt", Type::void_(), false });
        declare("assert_lt", { "assert_lt", Type::void_(), false });
        declare("assert_ge", { "assert_ge", Type::void_(), false });
        declare("assert_le", { "assert_le", Type::void_(), false });
        declare("assert_approx", { "assert_approx", Type::void_(), false });

        // Static constructor functions (Type::new() style)
        declare("HashMap::new", { "HashMap::new", Type::unknown(), false });
        declare("Vec::new", { "Vec::new", Type::unknown(), false });
        declare("Option::none", { "Option::none", Type::unknown(), false });
        declare("String::new", { "String::new", Type::string(), false });
        declare("Vec::with_capacity", { "Vec::with_capacity", Type::unknown(), false });
        declare("HashMap::with_capacity", { "HashMap::with_capacity", Type::unknown(), false });
        declare("HashSet::new", { "HashSet::new", Type::unknown(), false });
        declare("Deque::new", { "Deque::new", Type::unknown(), false });
        
        // Time functions
        builtin_functions_["time_now_ms"] = true;
        builtin_functions_["time_now_secs"] = true;
        builtin_functions_["sleep_ms"] = true;
        declare("time_now_ms", { "time_now_ms", Type::i32(), false });
        declare("time_now_secs", { "time_now_secs", Type::i32(), false });
        declare("sleep_ms", { "sleep_ms", Type::void_(), false });
        
        // Random functions
        builtin_functions_["random_int"] = true;
        declare("random_int", { "random_int", Type::i32(), false });
        
        // Path functions
        builtin_functions_["path_join"] = true;
        builtin_functions_["path_parent"] = true;
        builtin_functions_["path_filename"] = true;
        builtin_functions_["path_extension"] = true;
        builtin_functions_["is_directory"] = true;
        builtin_functions_["cwd"] = true;
        declare("path_join", { "path_join", Type::string(), false });
        declare("path_parent", { "path_parent", Type::string(), false });
        declare("path_filename", { "path_filename", Type::string(), false });
        declare("path_extension", { "path_extension", Type::string(), false });
        declare("is_directory", { "is_directory", Type::boolean(), false });
        declare("cwd", { "cwd", Type::string(), false });
        
        // Environment functions
        builtin_functions_["env_get"] = true;
        declare("env_get", { "env_get", Type::unknown(), false });  // returns Option<string>
        
        // Vec utilities
        builtin_functions_["vec_sort"] = true;
        builtin_functions_["vec_reverse"] = true;
        builtin_functions_["vec_contains"] = true;
        declare("vec_sort", { "vec_sort", Type::void_(), false });
        declare("vec_reverse", { "vec_reverse", Type::void_(), false });
        declare("vec_contains", { "vec_contains", Type::boolean(), false });
        
        // Format function
        builtin_functions_["format"] = true;
        declare("format", { "format", Type::string(), false });
    }

    void SemanticAnalyzer::analyze(AstModule* module) {
        push_scope();
        register_builtins();

        // First pass: register ALL function, struct, enum declarations
        // This enables forward references (calling functions defined later in the file)
        for (auto& d : module->decls)
            register_declaration(d.get());

        // Second pass: analyze all declaration bodies
        for (auto& d : module->decls)
            visit_decl(d.get());

        // Constant folding - evaluate constant expressions at compile time
        fold_constants_in_module(module);

        // Check for unused variables
        check_unused_variables();

        pop_scope();
    }

    void SemanticAnalyzer::register_declaration(AstDecl* d) {
        // Register function declarations (signature only, not body)
        if (auto fn = dynamic_cast<AstFuncDecl*>(d)) {
            Symbol sym;
            sym.name = fn->name;
            sym.type = parse_type_name(fn->return_type);
            sym.is_mutable = false;
            sym.is_public = fn->is_pub;
            sym.source_module = fn->source_module;
            sym.type_params = fn->type_params;

            for (const auto& c : fn->constraints) {
                sym.constraints.push_back({c.type_param, c.traits});
            }

            declare(fn->name, sym);
            func_decls_[fn->name] = fn;

            if (fn->is_test) {
                test_functions_.push_back(fn);
            }
            return;
        }

        // Register struct declarations
        if (auto s = dynamic_cast<AstStructDecl*>(d)) {
            Symbol sym;
            sym.name = s->name;
            sym.type = Type::struct_(s->name);
            sym.is_mutable = false;
            sym.is_public = s->is_pub;
            sym.source_module = s->source_module;
            declare(s->name, sym);
            struct_types_[s->name] = s;
            return;
        }

        // Register enum declarations
        if (auto e = dynamic_cast<AstEnumDecl*>(d)) {
            Symbol sym;
            sym.name = e->name;
            sym.type = Type::enum_(e->name);
            sym.is_mutable = false;
            sym.is_public = e->is_pub;
            sym.source_module = e->source_module;
            declare(e->name, sym);
            enum_types_[e->name] = e;
            return;
        }

        // Register trait declarations
        if (auto t = dynamic_cast<AstTraitDecl*>(d)) {
            Symbol sym;
            sym.name = t->name;
            sym.type = Type::unknown();  // Traits don't have a concrete type
            sym.is_mutable = false;
            sym.is_public = t->is_pub;
            sym.source_module = t->source_module;
            declare(t->name, sym);
            trait_types_[t->name] = t;
            return;
        }
    }

    void SemanticAnalyzer::push_scope() {
        scopes_.emplace_back();
    }

    void SemanticAnalyzer::pop_scope() {
        scopes_.pop_back();
    }

    bool SemanticAnalyzer::declare(const std::string& name, const Symbol& sym) {
        auto& scope = scopes_.back();
        if (scope.count(name)) return false;
        scope[name] = sym;
        return true;
    }

    Symbol* SemanticAnalyzer::lookup(const std::string& name) {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto f = it->find(name);
            if (f != it->end()) return &f->second;
        }
        return nullptr;
    }

    bool SemanticAnalyzer::check_visibility(const Symbol* sym, int line, int col) {
        if (!sym) return true;  // Not found is handled elsewhere

        // If symbol is from a different module and not public, error
        if (!sym->source_module.empty() &&
            sym->source_module != current_module_ &&
            !sym->is_public) {
            diag_.error("'" + sym->name + "' is private in module '" +
                       sym->source_module + "'", line, col);
            return false;
        }
        return true;
    }

    bool SemanticAnalyzer::type_implements_trait(const std::string& type_name, const std::string& trait_name) {
        // Check if we have recorded this implementation
        auto it = type_trait_impls_.find(type_name);
        if (it != type_trait_impls_.end()) {
            return it->second.count(trait_name) > 0;
        }

        // Built-in types implement certain traits implicitly
        static const std::unordered_map<std::string, std::set<std::string>> builtin_impls = {
            {"i32", {"Add", "Sub", "Mul", "Div", "Rem", "Eq", "Ord", "Copy", "Clone", "Default"}},
            {"i64", {"Add", "Sub", "Mul", "Div", "Rem", "Eq", "Ord", "Copy", "Clone", "Default"}},
            {"f32", {"Add", "Sub", "Mul", "Div", "Eq", "Copy", "Clone", "Default"}},
            {"f64", {"Add", "Sub", "Mul", "Div", "Eq", "Copy", "Clone", "Default"}},
            {"bool", {"Eq", "Copy", "Clone", "Default"}},
            {"string", {"Eq", "Clone", "Default", "Display"}},
        };

        auto builtin_it = builtin_impls.find(type_name);
        if (builtin_it != builtin_impls.end()) {
            return builtin_it->second.count(trait_name) > 0;
        }

        return false;
    }

    Type SemanticAnalyzer::instantiate_generic(const std::string& generic_type,
                                                const std::vector<Type>& type_args) {
        // This handles Vec<T>, Option<T>, Result<T,E> etc.
        if (generic_type == "Vec" && type_args.size() == 1) {
            Type t;
            t.kind = TypeKind::Array;
            t.element_type = type_args[0].name();
            return t;
        }
        if (generic_type == "Option" && type_args.size() == 1) {
            Type t;
            t.kind = TypeKind::Struct;
            t.struct_name = "Option<" + type_args[0].name() + ">";
            return t;
        }
        if (generic_type == "Result" && type_args.size() >= 1) {
            Type t;
            t.kind = TypeKind::Struct;
            t.struct_name = "Result<" + type_args[0].name();
            if (type_args.size() >= 2) {
                t.struct_name += ", " + type_args[1].name();
            }
            t.struct_name += ">";
            return t;
        }
        // For user-defined generic types
        Type t;
        t.kind = TypeKind::Struct;
        t.struct_name = generic_type + "<";
        for (size_t i = 0; i < type_args.size(); ++i) {
            if (i > 0) t.struct_name += ", ";
            t.struct_name += type_args[i].name();
        }
        t.struct_name += ">";
        return t;
    }

    bool SemanticAnalyzer::check_trait_bounds(const std::string& type_param,
                                               const Type& concrete_type,
                                               const std::vector<std::string>& required_traits,
                                               int line, int col) {
        std::string type_name = concrete_type.name();

        for (const auto& trait : required_traits) {
            if (!type_implements_trait(type_name, trait)) {
                diag_.error("type '" + type_name + "' does not implement trait '" +
                           trait + "' required by type parameter '" + type_param + "'",
                           line, col);
                return false;
            }
        }
        return true;
    }

    Type SemanticAnalyzer::parse_type_name(const std::string& name) {
        // Resolve type aliases first
        std::string resolved = name;
        if (type_aliases_.count(name)) {
            resolved = type_aliases_[name];
        }

        // Human-friendly type aliases (vNext)
        if (resolved == "int") resolved = "i64";
        if (resolved == "float") resolved = "f64";

        // All integer types map to I32 for type checking
        if (resolved == "i8" || resolved == "i16" || resolved == "i32" || resolved == "i64" ||
            resolved == "u8" || resolved == "u16" || resolved == "u32" || resolved == "u64")
            return Type::i32();
        // All float types map to F32 for type checking
        if (resolved == "f32" || resolved == "f64")
            return Type::f32();
        if (resolved == "bool") return Type::boolean();
        if (resolved == "string") return Type::string();
        if (resolved == "void") return Type::void_();
        if (resolved == "auto") return Type::unknown();

        // Dynamic trait type: dyn TraitName
        if (resolved.rfind("dyn ", 0) == 0) {
            // Return with the full name preserved for emitter
            Type t = Type::unknown();
            t.struct_name = resolved;  // Store the full type name
            return t;
        }

        // Pointer type: *T
        if (!resolved.empty() && resolved[0] == '*') {
            std::string pointee = resolved.substr(1);
            return Type::pointer(pointee);
        }

        // Reference type: &T or &mut T
        if (!resolved.empty() && resolved[0] == '&') {
            // Check for mutable reference: &mut T
            if (resolved.size() > 4 && resolved.substr(1, 4) == "mut ") {
                std::string referent = resolved.substr(5);
                return Type::mut_reference(referent);
            }
            std::string referent = resolved.substr(1);
            return Type::reference(referent);
        }

        // Array type: [N]T or []T
        if (!resolved.empty() && resolved[0] == '[') {
            size_t close = resolved.find(']');
            if (close != std::string::npos) {
                std::string size_str = resolved.substr(1, close - 1);
                std::string elem_type = resolved.substr(close + 1);
                int size = size_str.empty() ? 0 : std::stoi(size_str);
                return Type::array(elem_type, size);
            }
        }

        // Tuple type: (T1, T2, ...)
        if (!resolved.empty() && resolved[0] == '(' && resolved.back() == ')') {
            return Type::tuple(name);
        }

        // Handle generic types like Box<i32>, Vec<i32>, Result<T, E>, Option<T>
        std::string base_name = resolved;
        size_t angle = resolved.find('<');
        if (angle != std::string::npos) {
            base_name = resolved.substr(0, angle);
        }

        // Built-in generic types from standard library
        if (base_name == "Vec" || base_name == "Result" || base_name == "Option") {
            Type t = Type::unknown();
            t.struct_name = resolved;  // Store the full type name
            return t;
        }

        // Check if it's a struct type
        if (struct_types_.count(base_name)) return Type::struct_(name);
        // Check if it's an enum type
        if (enum_types_.count(base_name)) return Type::enum_(name);
        return Type::unknown();
    }

    bool SemanticAnalyzer::is_numeric(const Type& t) {
        return t.kind == TypeKind::I32 || t.kind == TypeKind::F32;
    }

    bool SemanticAnalyzer::always_returns(const AstStmt* stmt) {
        if (!stmt) return false;

        // Return statement always returns
        if (stmt->kind == NodeKind::ReturnStmt) return true;

        // Block: check if any statement in the block always returns
        if (auto block = dynamic_cast<const AstBlockStmt*>(stmt)) {
            for (const auto& s : block->statements) {
                if (always_returns(s.get())) return true;
            }
            return false;
        }

        // If statement: returns if both branches always return
        if (auto if_stmt = dynamic_cast<const AstIfStmt*>(stmt)) {
            if (!if_stmt->else_block) return false;  // No else means might not return
            return always_returns(if_stmt->then_block.get()) &&
                   always_returns(if_stmt->else_block.get());
        }

        // Loop: infinite loop returns if body always returns (conservative)
        if (auto loop = dynamic_cast<const AstLoopStmt*>(stmt)) {
            // Infinite loop without break is considered to always "return" (never exits)
            // But for simplicity, we'll be conservative
            return false;
        }

        // Match expression (when used as statement): returns if all arms return
        // For now, be conservative
        return false;
    }

    std::string SemanticAnalyzer::infer_type_name(const Type& t) {
        switch (t.kind) {
            case TypeKind::I32: return "i32";
            case TypeKind::F32: return "f32";
            case TypeKind::Bool: return "bool";
            case TypeKind::String: return "string";
            case TypeKind::Void: return "void";
            case TypeKind::Struct: return t.struct_name;
            case TypeKind::Enum: return t.struct_name;
            case TypeKind::Pointer: return "*" + t.element_type;
            case TypeKind::Reference: return "&" + t.element_type;
            case TypeKind::MutReference: return "&mut " + t.element_type;
            case TypeKind::Tuple: return t.struct_name;  // Already in "(T1, T2, ...)" format
            case TypeKind::Array:
                if (t.array_size > 0) {
                    return "[" + std::to_string(t.array_size) + "]" + t.element_type;
                } else {
                    return "[]" + t.element_type;
                }
            case TypeKind::Unknown:
                // If struct_name is set, use it (for generic types like Vec<i32>)
                if (!t.struct_name.empty()) return t.struct_name;
                return "auto";
            default: return "auto";
        }
    }

    // -------- declarations --------

    void SemanticAnalyzer::visit_decl(AstDecl* d) {
        // Handle use declarations - register imported symbols
        if (auto use = dynamic_cast<AstUseDecl*>(d)) {
            // For now, just record the import - in a full implementation
            // we would load the module and register its exported symbols
            imported_modules_.push_back(use->module_path);
            
            // If specific names are imported, mark them as available
            for (const auto& name : use->imported_names) {
                // Register as an unknown type symbol for now
                declare(name, { name, Type::unknown(), false });
            }
            return;
        }

        if (auto fn = dynamic_cast<AstFuncDecl*>(d)) {
            // Declaration was already registered in register_declaration()
            // Here we only analyze the function body

            // Validate where clause constraints
            for (const auto& constraint : fn->constraints) {
                // Check type param exists
                bool found = false;
                for (const auto& tp : fn->type_params) {
                    if (tp == constraint.type_param) { found = true; break; }
                }
                if (!found) {
                    diag_.error("where clause references unknown type parameter '" + 
                               constraint.type_param + "'", constraint.line, constraint.column);
                }
                
                // Check traits exist
                for (const auto& trait_name : constraint.traits) {
                    if (!trait_types_.count(trait_name)) {
                        diag_.error("where clause references unknown trait '" + 
                                   trait_name + "'", constraint.line, constraint.column);
                    }
                }
            }

            push_scope();

            // For methods, add 'self' to scope
            if (fn->is_method()) {
                Type receiver = parse_type_name(fn->receiver_type);
                current_receiver_type_ = receiver;
                declare("self", { "self", receiver, true });
            }

            for (auto& p : fn->params) {
                declare(p.name, { p.name, parse_type_name(p.type_name), true });
            }

            current_return_type_ = parse_type_name(fn->return_type);
            visit_stmt(fn->body.get());

            // Check return coverage for non-void functions (except main which gets implicit return 0)
            if (current_return_type_.kind != TypeKind::Void &&
                !(fn->name == "main" && !fn->is_method())) {
                if (!always_returns(fn->body.get())) {
                    diag_.error("function '" + fn->name + "' does not return a value on all code paths",
                               fn->line, fn->column);
                }
            }

            pop_scope();
            current_receiver_type_ = Type::unknown();
            return;
        }

        if (auto g = dynamic_cast<AstGlobalVarDecl*>(d)) {
            auto* v = g->var.get();
            Type t = parse_type_name(v->type_name);
            declare(v->name, { v->name, t, true });
            if (v->init_expr)
                visit_expr(static_cast<AstExpr*>(v->init_expr.get()));
            return;
        }

        if (auto s = dynamic_cast<AstStructDecl*>(d)) {
            // Declaration was already registered in register_declaration()
            // Here we only validate default values for fields
            for (auto& field : s->fields) {
                if (field.default_value) {
                    Type expected = parse_type_name(field.type_name);
                    Type actual = visit_expr(field.default_value.get());
                    if (actual != expected && actual.kind != TypeKind::Unknown && expected.kind != TypeKind::Unknown) {
                        diag_.error("default value type mismatch for field '" + field.name + "'", 
                                   field.line, field.column);
                    }
                }
            }
            return;
        }

        if (auto e = dynamic_cast<AstEnumDecl*>(d)) {
            // Declaration was already registered in register_declaration()
            // No additional analysis needed for enums
            return;
        }

        if (auto t = dynamic_cast<AstTraitDecl*>(d)) {
            // Declaration was already registered in register_declaration()
            // No additional analysis needed for traits
            return;
        }

        if (auto impl = dynamic_cast<AstImplDecl*>(d)) {
            // Check that the type exists
            if (!struct_types_.count(impl->type_name) && !enum_types_.count(impl->type_name)) {
                diag_.error("impl for unknown type", impl->line, impl->column);
                return;
            }

            // If implementing a trait, check it exists and validate associated types
            if (impl->is_trait_impl()) {
                // Allow operator traits (they are built-in)
                if (!trait_types_.count(impl->trait_name) && !is_operator_trait(impl->trait_name)) {
                    diag_.error("impl for unknown trait", impl->line, impl->column);
                    return;
                }

                // Validate associated types (skip for built-in operator traits)
                if (!is_operator_trait(impl->trait_name)) {
                const auto* trait = trait_types_.at(impl->trait_name);
                std::unordered_set<std::string> provided_types;
                for (const auto& ta : impl->type_assignments) {
                    provided_types.insert(ta.name);
                }

                // Check that all required associated types are provided
                for (const auto& at : trait->associated_types) {
                    if (!provided_types.count(at.name)) {
                        diag_.error("missing associated type '" + at.name + 
                                   "' in impl for " + impl->trait_name, 
                                   impl->line, impl->column);
                    }
                }

                // Check for extra associated types
                std::unordered_set<std::string> declared_types;
                for (const auto& at : trait->associated_types) {
                    declared_types.insert(at.name);
                }
                for (const auto& ta : impl->type_assignments) {
                    if (!declared_types.count(ta.name)) {
                        diag_.error("unknown associated type '" + ta.name + 
                                   "' in impl for " + impl->trait_name, 
                                   ta.line, ta.column);
                    }
                }
                }  // end if (!is_operator_trait)

                // Record that this type implements this trait
                type_trait_impls_[impl->type_name].insert(impl->trait_name);
            }

            // Register constants in the impl block
            for (const auto& c : impl->constants) {
                Symbol sym{ c.name, parse_type_name(c.type_name), false };
                std::string qualified_name = impl->type_name + "_" + c.name;
                declare(qualified_name, sym);
            }

            // Analyze each method in the impl block
            for (auto& method : impl->methods) {
                Symbol sym{ method->name, parse_type_name(method->return_type), false };
                std::string qualified_name = impl->type_name + "_" + method->name;
                declare(qualified_name, sym);
                func_decls_[qualified_name] = method.get();  // Store for named argument validation

                push_scope();

                // Add 'self' to scope only for non-static methods
                if (!method->is_static) {
                    Type receiver = parse_type_name(impl->type_name);
                    current_receiver_type_ = receiver;
                    declare("self", { "self", receiver, true });
                }

                for (auto& p : method->params) {
                    declare(p.name, { p.name, parse_type_name(p.type_name), true });
                }

                current_return_type_ = sym.type;
                visit_stmt(method->body.get());

                pop_scope();
                current_receiver_type_ = Type::unknown();
            }
            return;
        }

        if (auto alias = dynamic_cast<AstTypeAliasDecl*>(d)) {
            // Register the type alias
            if (type_aliases_.count(alias->alias_name)) {
                diag_.error("type alias already defined: " + alias->alias_name, alias->line, alias->column);
                return;
            }
            type_aliases_[alias->alias_name] = alias->target_type;
            return;
        }
    }

    // -------- statements --------

    void SemanticAnalyzer::visit_stmt(AstStmt* s) {
        if (auto b = dynamic_cast<AstBlockStmt*>(s)) {
            push_scope();
            bool has_terminator = false;
            int terminator_line = 0;
            for (auto& st : b->statements) {
                if (has_terminator) {
                    // Dead code warning - this statement is unreachable
                    diag_.warning("unreachable code after " +
                        (terminator_line > 0 ? "line " + std::to_string(terminator_line) : "terminating statement"),
                        st->line, st->column);
                    // Only warn once per block
                    break;
                }
                visit_stmt(st.get());

                // Check if this statement is a terminator
                if (dynamic_cast<AstReturnStmt*>(st.get()) ||
                    dynamic_cast<AstBreakStmt*>(st.get()) ||
                    dynamic_cast<AstContinueStmt*>(st.get())) {
                    has_terminator = true;
                    terminator_line = st->line;
                }
            }
            pop_scope();
            return;
        }

        if (auto v = dynamic_cast<AstVarDeclStmt*>(s)) {
            Type t = parse_type_name(v->type_name);
            if (v->init_expr) {
                Type rhs = visit_expr(static_cast<AstExpr*>(v->init_expr.get()));
                // Type inference: if type is "auto" or empty, use the RHS type
                if (v->type_name == "auto" || v->type_name.empty()) {
                    t = rhs;
                    // Update the AST node's type for code generation
                    v->type_name = infer_type_name(rhs);
                }
                else if (t != rhs && rhs.kind != TypeKind::Unknown && t.kind != TypeKind::Unknown) {
                    diag_.error("type mismatch in variable initialization: expected " + t.name() + ", got " + rhs.name(), s->line, s->column);
                }
            }
            // Track mutability from the variable declaration
            declare(v->name, { v->name, t, v->is_mutable });
            // Track for unused variable warning
            variable_used_[v->name] = false;
            variable_location_[v->name] = {s->line, s->column};
            return;
        }

        if (auto a = dynamic_cast<AstAssignStmt*>(s)) {
            Type target_type;
            if (a->target_expr) {
                // Complex target (member access or index)
                target_type = visit_expr(static_cast<AstExpr*>(a->target_expr.get()));
            } else {
                auto* sym = lookup(a->target_name);
                if (!sym) {
                    std::string suggestion = find_similar_name(a->target_name);
                    std::string msg = "assignment to undeclared variable '" + a->target_name + "'";
                    if (!suggestion.empty()) msg += "; did you mean '" + suggestion + "'?";
                    diag_.error(msg, s->line, s->column);
                    return;
                }
                // Check mutability
                if (!sym->is_mutable) {
                    diag_.error("cannot assign to immutable variable '" + a->target_name + "'", s->line, s->column);
                    return;
                }
                target_type = sym->type;
            }
            Type rhs = visit_expr(static_cast<AstExpr*>(a->value.get()));
            if (rhs != target_type && rhs.kind != TypeKind::Unknown && target_type.kind != TypeKind::Unknown)
                diag_.error("type mismatch in assignment: expected " + target_type.name() + ", got " + rhs.name(), s->line, s->column);
            return;
        }

        if (auto i = dynamic_cast<AstIfStmt*>(s)) {
            if (i->is_if_let) {
                // if-let pattern matching
                Type expr_type = visit_expr(static_cast<AstExpr*>(i->pattern_expr.get()));

                // Determine the inner type based on pattern and expr type
                Type inner_type = Type::unknown();
                if (i->pattern_kind == "Some") {
                    // Option<T> -> T
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Option<", 0) == 0) {
                        size_t start = 7;
                        size_t end = expr_type.struct_name.size() - 1;
                        inner_type = parse_type_name(expr_type.struct_name.substr(start, end - start));
                    }
                } else if (i->pattern_kind == "Ok") {
                    // Result<T, E> -> T
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Result<", 0) == 0) {
                        size_t start = 7;
                        size_t comma = expr_type.struct_name.find(',', start);
                        if (comma != std::string::npos) {
                            inner_type = parse_type_name(expr_type.struct_name.substr(start, comma - start));
                        }
                    }
                } else if (i->pattern_kind == "Err") {
                    // Result<T, E> -> E
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Result<", 0) == 0) {
                        size_t comma = expr_type.struct_name.find(',');
                        if (comma != std::string::npos) {
                            size_t start = comma + 1;
                            while (start < expr_type.struct_name.size() && expr_type.struct_name[start] == ' ') start++;
                            size_t end = expr_type.struct_name.size() - 1;
                            inner_type = parse_type_name(expr_type.struct_name.substr(start, end - start));
                        }
                    }
                }
                // None pattern doesn't bind a variable

                // Create a new scope for the then block with the bound variable
                push_scope();
                if (!i->pattern_var.empty()) {
                    Symbol sym;
                    sym.name = i->pattern_var;
                    sym.type = inner_type;
                    sym.is_mutable = false;  // if-let bindings are immutable by default
                    declare(i->pattern_var, sym);
                }
                visit_stmt(i->then_block.get());
                pop_scope();
            } else {
                Type c = visit_expr(static_cast<AstExpr*>(i->condition.get()));
                if (c.kind != TypeKind::Bool)
                    diag_.error("if condition must be bool, got " + c.name(), s->line, s->column);
                visit_stmt(i->then_block.get());
            }
            if (i->else_block) visit_stmt(i->else_block.get());
            return;
        }

        if (auto w = dynamic_cast<AstWhileStmt*>(s)) {
            loop_depth_++;
            if (w->is_while_let) {
                // while-let pattern matching
                Type expr_type = visit_expr(static_cast<AstExpr*>(w->pattern_expr.get()));

                // Determine the inner type based on pattern and expr type
                Type inner_type = Type::unknown();
                if (w->pattern_kind == "Some") {
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Option<", 0) == 0) {
                        size_t start = 7;
                        size_t end = expr_type.struct_name.size() - 1;
                        inner_type = parse_type_name(expr_type.struct_name.substr(start, end - start));
                    }
                } else if (w->pattern_kind == "Ok") {
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Result<", 0) == 0) {
                        size_t start = 7;
                        size_t comma = expr_type.struct_name.find(',', start);
                        if (comma != std::string::npos) {
                            inner_type = parse_type_name(expr_type.struct_name.substr(start, comma - start));
                        }
                    }
                } else if (w->pattern_kind == "Err") {
                    if (expr_type.kind == TypeKind::Struct && expr_type.struct_name.rfind("Result<", 0) == 0) {
                        size_t comma = expr_type.struct_name.find(',');
                        if (comma != std::string::npos) {
                            size_t start = comma + 1;
                            while (start < expr_type.struct_name.size() && expr_type.struct_name[start] == ' ') start++;
                            size_t end = expr_type.struct_name.size() - 1;
                            inner_type = parse_type_name(expr_type.struct_name.substr(start, end - start));
                        }
                    }
                }

                // Create scope with bound variable
                push_scope();
                if (!w->pattern_var.empty()) {
                    Symbol sym;
                    sym.name = w->pattern_var;
                    sym.type = inner_type;
                    sym.is_mutable = false;
                    declare(w->pattern_var, sym);
                }
                visit_stmt(w->body.get());
                pop_scope();
            } else {
                Type c = visit_expr(static_cast<AstExpr*>(w->condition.get()));
                if (c.kind != TypeKind::Bool)
                    diag_.error("while condition must be bool, got " + c.name(), s->line, s->column);
                visit_stmt(w->body.get());
            }
            loop_depth_--;
            return;
        }

        if (dynamic_cast<AstBreakStmt*>(s)) {
            if (loop_depth_ == 0)
                diag_.error("break outside loop", s->line, s->column);
            return;
        }

        if (dynamic_cast<AstContinueStmt*>(s)) {
            if (loop_depth_ == 0)
                diag_.error("continue outside loop", s->line, s->column);
            return;
        }

        if (auto r = dynamic_cast<AstReturnStmt*>(s)) {
            if (r->value) {
                Type v = visit_expr(static_cast<AstExpr*>(r->value.get()));
                // Allow unknown types (e.g., from method calls like unwrap())
                if (v != current_return_type_ && v.kind != TypeKind::Unknown && current_return_type_.kind != TypeKind::Unknown)
                    diag_.error("return type mismatch: expected " + current_return_type_.name() + ", got " + v.name(), s->line, s->column);
            }
            return;
        }

        if (auto d = dynamic_cast<AstDeferStmt*>(s)) {
            visit_stmt(d->body.get());
            return;
        }

        if (auto e = dynamic_cast<AstExprStmt*>(s)) {
            visit_expr(static_cast<AstExpr*>(e->expr.get()));
            return;
        }

        if (auto ds = dynamic_cast<AstDestructureStmt*>(s)) {
            // Visit the init expression and get its type
            Type init_type = Type::unknown();
            if (ds->init_expr) {
                init_type = visit_expr(static_cast<AstExpr*>(ds->init_expr.get()));
            }

            // Use init type for tuple destructuring when type is "auto"
            Type base_type = (ds->type_name == "auto") ? init_type : parse_type_name(ds->type_name);

            // For tuple destructuring, parse element types from the tuple type
            std::vector<std::string> tuple_elem_types;
            if (ds->is_tuple && base_type.kind == TypeKind::Tuple) {
                // Parse "(T1, T2, T3)" -> ["T1", "T2", "T3"]
                std::string inner = base_type.struct_name.substr(1, base_type.struct_name.size() - 2);
                size_t start = 0;
                int depth = 0;
                for (size_t i = 0; i <= inner.size(); ++i) {
                    char c = (i < inner.size()) ? inner[i] : ',';
                    if (c == '(' || c == '<') depth++;
                    else if (c == ')' || c == '>') depth--;
                    else if ((c == ',' || i == inner.size()) && depth == 0) {
                        std::string elem = inner.substr(start, i - start);
                        while (!elem.empty() && elem.front() == ' ') elem.erase(0, 1);
                        while (!elem.empty() && elem.back() == ' ') elem.pop_back();
                        if (!elem.empty()) tuple_elem_types.push_back(elem);
                        start = i + 1;
                    }
                }
            }

            for (size_t i = 0; i < ds->bindings.size(); ++i) {
                auto& binding = ds->bindings[i];
                Type binding_type = Type::unknown();

                if (ds->is_tuple) {
                    // Tuple destructuring: use element type at index
                    if (i < tuple_elem_types.size()) {
                        binding_type = parse_type_name(tuple_elem_types[i]);
                    }
                } else if (ds->is_struct && struct_types_.count(ds->type_name)) {
                    // Struct destructuring: look up field type
                    auto* struct_decl = struct_types_[ds->type_name];
                    for (auto& field : struct_decl->fields) {
                        if (field.name == binding.field_name) {
                            binding_type = parse_type_name(field.type_name);
                            break;
                        }
                    }
                } else if (!ds->is_struct && base_type.kind == TypeKind::Array) {
                    // Array element type from the array type
                    binding_type = parse_type_name(base_type.element_type);
                }

                declare(binding.name, { binding.name, binding_type, true });
            }
            return;
        }

        if (auto fin = dynamic_cast<AstForInStmt*>(s)) {
            // Visit the iterable
            if (fin->iterable) {
                visit_expr(static_cast<AstExpr*>(fin->iterable.get()));
            }

            // Enter new scope for loop variable(s)
            push_scope();

            if (fin->is_destructure) {
                // Declare all destructured variables (types inferred, use auto-like approach)
                for (const auto& name : fin->var_names) {
                    declare(name, { name, Type::i32(), true });  // Type will be inferred at runtime
                }
            } else {
                // Declare single loop variable (type inferred from iterable, use i32 for now)
                declare(fin->var_name, { fin->var_name, Type::i32(), true });
            }

            loop_depth_++;
            visit_stmt(fin->body.get());
            loop_depth_--;

            pop_scope();
            return;
        }
    }

    // -------- expressions --------

    Type SemanticAnalyzer::visit_expr(AstExpr* e) {
        if (auto l = dynamic_cast<AstLiteralExpr*>(e)) {
            if (l->value == "true" || l->value == "false") return Type::boolean();
            if (l->is_string) return Type::string();  // Use is_string flag
            if (l->value.find('.') != std::string::npos) return Type::f32();
            return Type::i32();
        }

        if (auto id = dynamic_cast<AstIdentifierExpr*>(e)) {
            auto* sym = lookup(id->name);
            if (!sym) {
                std::string suggestion = find_similar_name(id->name);
                std::string msg = "use of undeclared identifier '" + id->name + "'";
                if (!suggestion.empty()) msg += "; did you mean '" + suggestion + "'?";
                diag_.error(msg, e->line, e->column);
                return Type::unknown();
            }
            mark_variable_used(id->name);
            return sym->type;
        }

        if (auto b = dynamic_cast<AstBinaryExpr*>(e)) {
            Type L = visit_expr(static_cast<AstExpr*>(b->left.get()));
            Type R = visit_expr(static_cast<AstExpr*>(b->right.get()));

            // Comparison operators always return bool
            if (b->op == "==" || b->op == "!=" ||
                b->op == "<" || b->op == "<=" ||
                b->op == ">" || b->op == ">=")
                return Type::boolean();

            // Boolean operators: && and || require bool operands
            if (b->op == "&&" || b->op == "||") {
                if (L.kind != TypeKind::Bool) {
                    diag_.error("left operand of '" + b->op + "' must be bool, got " + L.name(), e->line, e->column);
                    return Type::unknown();
                }
                if (R.kind != TypeKind::Bool) {
                    diag_.error("right operand of '" + b->op + "' must be bool, got " + R.name(), e->line, e->column);
                    return Type::unknown();
                }
                return Type::boolean();
            }

            // Numeric operations
            if (is_numeric(L) && is_numeric(R))
                return L;

            // String concatenation
            if (b->op == "+" && L.kind == TypeKind::String && R.kind == TypeKind::String)
                return Type::string();

            diag_.error("invalid binary operator operands: cannot apply '" + b->op + "' to " + L.name() + " and " + R.name(), e->line, e->column);
            return Type::unknown();
        }

        if (auto u = dynamic_cast<AstUnaryExpr*>(e)) {
            Type operand = visit_expr(static_cast<AstExpr*>(u->right.get()));

            // Immutable reference: &x returns &T
            if (u->op == "&") {
                return Type::reference(operand.name());
            }

            // Mutable reference: &mut x returns &mut T
            if (u->op == "&mut") {
                return Type::mut_reference(operand.name());
            }

            // Dereference operator: *ptr returns T
            if (u->op == "*") {
                if (operand.kind == TypeKind::Pointer) {
                    return parse_type_name(operand.element_type);
                }
                if (operand.kind == TypeKind::Reference || operand.kind == TypeKind::MutReference) {
                    return parse_type_name(operand.element_type);
                }
                diag_.error("cannot dereference non-pointer type " + operand.name(), e->line, e->column);
                return Type::unknown();
            }

            return operand;
        }

        if (auto c = dynamic_cast<AstCallExpr*>(e)) {
            std::string lookup_name = c->func_name;
            // Transform Type::method to Type_method for static method calls
            size_t pos = lookup_name.find("::");
            if (pos != std::string::npos) {
                lookup_name = lookup_name.substr(0, pos) + "_" + lookup_name.substr(pos + 2);
            }
            auto* sym = lookup(lookup_name);
            // Check visibility - reject private symbols from other modules
            if (sym) {
                check_visibility(sym, e->line, e->column);
            }
            if (!sym) {
                // Check if this is an enum variant constructor (EnumName::VariantName)
                size_t sep = c->func_name.find("::");
                if (sep != std::string::npos) {
                    std::string enum_name = c->func_name.substr(0, sep);
                    std::string variant_name = c->func_name.substr(sep + 2);
                    if (enum_types_.count(enum_name)) {
                        auto* ed = enum_types_[enum_name];
                        for (const auto& v : ed->variants) {
                            if (v.name == variant_name) {
                                // This is a valid enum variant constructor
                                // Check argument count for tuple variants
                                if (v.is_tuple_variant()) {
                                    if (c->args.size() != v.tuple_types.size()) {
                                        diag_.error("wrong number of arguments for enum variant '" + variant_name + "': expected " + std::to_string(v.tuple_types.size()) + ", got " + std::to_string(c->args.size()), e->line, e->column);
                                        return Type::unknown();
                                    }
                                    // Analyze each argument
                                    for (auto& arg : c->args) {
                                        visit_expr(static_cast<AstExpr*>(arg.get()));
                                    }
                                } else if (v.is_struct_variant()) {
                                    if (c->args.size() != v.struct_fields.size()) {
                                        diag_.error("wrong number of arguments for enum variant", e->line, e->column);
                                        return Type::unknown();
                                    }
                                    // Analyze each argument
                                    for (auto& arg : c->args) {
                                        visit_expr(static_cast<AstExpr*>(arg.get()));
                                    }
                                } else if (!c->args.empty()) {
                                    diag_.error("unit variant '" + variant_name + "' takes no arguments", e->line, e->column);
                                    return Type::unknown();
                                }
                                return Type::enum_(enum_name);
                            }
                        }
                        diag_.error("unknown variant '" + variant_name + "' for enum '" + enum_name + "'", e->line, e->column);
                        return Type::unknown();
                    }
                }
                std::string suggestion = find_similar_name(c->func_name);
                std::string msg = "call to undeclared function '" + c->func_name + "'";
                if (!suggestion.empty()) msg += "; did you mean '" + suggestion + "'?";
                diag_.error(msg, e->line, e->column);
                return Type::unknown();
            }
            
            // Handle named arguments - validate and reorder
            bool has_named_args = false;
            for (const auto& name : c->arg_names) {
                if (!name.empty()) { has_named_args = true; break; }
            }
            
            if (has_named_args && func_decls_.count(lookup_name)) {
                auto* fn = func_decls_[lookup_name];
                std::vector<std::unique_ptr<AstNode>> reordered_args;
                std::vector<bool> param_filled(fn->params.size(), false);
                
                // First pass: place positional arguments
                size_t pos_idx = 0;
                for (size_t i = 0; i < c->args.size() && pos_idx < fn->params.size(); ++i) {
                    if (c->arg_names[i].empty()) {
                        // Positional argument
                        while (pos_idx < fn->params.size() && param_filled[pos_idx]) pos_idx++;
                        if (pos_idx < fn->params.size()) {
                            param_filled[pos_idx] = true;
                            pos_idx++;
                        }
                    }
                }
                
                // Create reordered args vector
                reordered_args.resize(fn->params.size());
                pos_idx = 0;
                for (size_t i = 0; i < c->args.size(); ++i) {
                    if (c->arg_names[i].empty()) {
                        // Positional argument - find next unfilled slot
                        while (pos_idx < fn->params.size() && reordered_args[pos_idx]) pos_idx++;
                        if (pos_idx < fn->params.size()) {
                            reordered_args[pos_idx] = std::move(c->args[i]);
                            pos_idx++;
                        }
                    } else {
                        // Named argument - find matching parameter
                        bool found = false;
                        for (size_t j = 0; j < fn->params.size(); ++j) {
                            if (fn->params[j].name == c->arg_names[i]) {
                                if (reordered_args[j]) {
                                    diag_.error("duplicate argument for parameter '" + c->arg_names[i] + "'", e->line, e->column);
                                } else {
                                    reordered_args[j] = std::move(c->args[i]);
                                }
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            diag_.error("unknown parameter name '" + c->arg_names[i] + "'", e->line, e->column);
                        }
                    }
                }
                
                // Replace args with reordered version
                c->args = std::move(reordered_args);
                c->arg_names.clear();  // Clear names after reordering
            }
            
            // Collect argument types for generic constraint checking
            std::vector<Type> arg_types;
            for (auto& a : c->args) {
                if (a) arg_types.push_back(visit_expr(static_cast<AstExpr*>(a.get())));
            }

            // Infer generic type arguments from argument types
            std::unordered_map<std::string, Type> type_bindings;
            if (func_decls_.count(lookup_name)) {
                auto* fn = func_decls_[lookup_name];
                if (!fn->type_params.empty()) {
                    // Build mapping from type params to concrete types
                    for (size_t i = 0; i < fn->params.size() && i < arg_types.size(); ++i) {
                        const std::string& param_type = fn->params[i].type_name;
                        // If parameter type is a type param, bind it
                        for (const auto& tp : fn->type_params) {
                            if (param_type == tp && type_bindings.find(tp) == type_bindings.end()) {
                                type_bindings[tp] = arg_types[i];
                                break;
                            }
                            // Also handle generic containers like Vec<T>, Option<T>
                            size_t pos = param_type.find('<');
                            if (pos != std::string::npos) {
                                size_t end = param_type.rfind('>');
                                if (end != std::string::npos && end > pos) {
                                    std::string inner_type = param_type.substr(pos + 1, end - pos - 1);
                                    if (inner_type == tp) {
                                        // Extract inner type from arg (e.g., Vec<i32> -> i32)
                                        std::string arg_name = arg_types[i].name();
                                        size_t arg_pos = arg_name.find('<');
                                        size_t arg_end = arg_name.rfind('>');
                                        if (arg_pos != std::string::npos && arg_end != std::string::npos) {
                                            std::string arg_inner = arg_name.substr(arg_pos + 1, arg_end - arg_pos - 1);
                                            type_bindings[tp] = parse_type_name(arg_inner);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Validate generic constraints if function has where clauses
            if (!sym->constraints.empty()) {
                // Check each constraint
                for (const auto& constraint : sym->constraints) {
                    const std::string& type_param = constraint.first;
                    const std::vector<std::string>& required_traits = constraint.second;

                    auto it = type_bindings.find(type_param);
                    if (it != type_bindings.end()) {
                        check_trait_bounds(type_param, it->second, required_traits, e->line, e->column);
                    }
                }
            }
            
            // Substitute type parameters in return type
            if (!type_bindings.empty() && func_decls_.count(lookup_name)) {
                auto* fn = func_decls_[lookup_name];
                std::string ret_type = fn->return_type;
                // Check if return type is a type parameter
                auto it = type_bindings.find(ret_type);
                if (it != type_bindings.end()) {
                    return it->second;
                }
                // Check for generic containers in return type
                for (const auto& [tp, concrete] : type_bindings) {
                    size_t pos = 0;
                    while ((pos = ret_type.find(tp, pos)) != std::string::npos) {
                        // Make sure it's a whole word match
                        bool before_ok = (pos == 0 || !isalnum(ret_type[pos-1]));
                        bool after_ok = (pos + tp.size() >= ret_type.size() || 
                                        !isalnum(ret_type[pos + tp.size()]));
                        if (before_ok && after_ok) {
                            ret_type.replace(pos, tp.size(), concrete.name());
                            pos += concrete.name().size();
                        } else {
                            pos++;
                        }
                    }
                }
                if (ret_type != fn->return_type) {
                    return parse_type_name(ret_type);
                }
            }
            return sym->type;
        }

        if (auto m = dynamic_cast<AstMethodCallExpr*>(e)) {
            Type obj_type = visit_expr(m->object.get());

            // Store the type for code generation
            if (obj_type.kind == TypeKind::Struct) {
                m->object_type = obj_type.struct_name;
            }
            // Also handle enum types explicitly
            if (obj_type.kind == TypeKind::Enum) {
                m->object_type = obj_type.struct_name;
            }
            // Store type name for dyn trait types (for -> vs . in emitter)
            if (!obj_type.name().empty()) {
                m->object_type = obj_type.name();
            }

            // Handle named arguments - validate and reorder
            bool has_named_args = false;
            for (const auto& name : m->arg_names) {
                if (!name.empty()) { has_named_args = true; break; }
            }
            
            if (has_named_args && !m->object_type.empty()) {
                std::string qualified_name = m->object_type + "_" + m->method_name;
                if (func_decls_.count(qualified_name)) {
                    auto* fn = func_decls_[qualified_name];
                    // Note: 'self' is implicit and NOT in params list
                    size_t param_count = fn->params.size();
                    
                    std::vector<std::unique_ptr<AstNode>> reordered_args;
                    reordered_args.resize(param_count);
                    size_t pos_idx = 0;
                    
                    for (size_t i = 0; i < m->args.size(); ++i) {
                        if (m->arg_names[i].empty()) {
                            // Positional argument
                            while (pos_idx < param_count && reordered_args[pos_idx]) pos_idx++;
                            if (pos_idx < param_count) {
                                reordered_args[pos_idx] = std::move(m->args[i]);
                                pos_idx++;
                            }
                        } else {
                            // Named argument - find matching parameter
                            bool found = false;
                            for (size_t j = 0; j < fn->params.size(); ++j) {
                                if (fn->params[j].name == m->arg_names[i]) {
                                    if (reordered_args[j]) {
                                        diag_.error("duplicate argument for parameter '" + m->arg_names[i] + "'", e->line, e->column);
                                    } else {
                                        reordered_args[j] = std::move(m->args[i]);
                                    }
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                diag_.error("unknown parameter name '" + m->arg_names[i] + "'", e->line, e->column);
                            }
                        }
                    }
                    
                    m->args = std::move(reordered_args);
                    m->arg_names.clear();
                }
            }

            for (auto& a : m->args) {
                if (a) visit_expr(static_cast<AstExpr*>(a.get()));
            }

            // Handle common Result/Option methods
            if (m->method_name == "is_ok" || m->method_name == "is_err" ||
                m->method_name == "is_some" || m->method_name == "is_none") {
                return Type::boolean();
            }

            // unwrap returns the inner type - for now return unknown
            // TODO: look up the method and return its return type
            return Type::unknown();
        }

        if (auto idx = dynamic_cast<AstIndexExpr*>(e)) {
            Type base = visit_expr(idx->base.get());
            visit_expr(idx->index.get());
            // For now, just return unknown for array element type
            return Type::unknown();
        }

        if (auto m = dynamic_cast<AstMemberAccessExpr*>(e)) {
            Type obj_type = visit_expr(m->object.get());

            // Handle references by looking through to the referent type
            if (obj_type.kind == TypeKind::Reference || obj_type.kind == TypeKind::MutReference) {
                obj_type = parse_type_name(obj_type.element_type);
            }

            if (obj_type.kind == TypeKind::Struct) {
                auto it = struct_types_.find(obj_type.struct_name);
                if (it != struct_types_.end()) {
                    for (auto& field : it->second->fields) {
                        if (field.name == m->member_name) {
                            return parse_type_name(field.type_name);
                        }
                    }
                    diag_.error("unknown struct member '" + m->member_name + "' on type " + obj_type.name(), e->line, e->column);
                }
            }
            return Type::unknown();
        }

        if (auto arr = dynamic_cast<AstArrayLiteralExpr*>(e)) {
            // Infer array type from elements
            if (arr->elements.empty()) {
                return Type::unknown();  // Empty array needs explicit type
            }

            // Get the type of the first element
            Type elem_type = visit_expr(arr->elements[0].get());

            // Check that all elements have compatible types
            for (size_t i = 1; i < arr->elements.size(); ++i) {
                Type t = visit_expr(arr->elements[i].get());
                if (t != elem_type && t.kind != TypeKind::Unknown && elem_type.kind != TypeKind::Unknown) {
                    diag_.error("array elements have inconsistent types: " + elem_type.name() + " vs " + t.name(), e->line, e->column);
                }
            }

            // Return array type with element type and size
            return Type::array(infer_type_name(elem_type), static_cast<int>(arr->elements.size()));
        }

        // Tuple literal: (expr1, expr2, ...)
        if (auto t = dynamic_cast<AstTupleExpr*>(e)) {
            std::string type_str = "(";
            for (size_t i = 0; i < t->elements.size(); ++i) {
                if (i > 0) type_str += ", ";
                Type elem_type = visit_expr(t->elements[i].get());
                type_str += infer_type_name(elem_type);
            }
            type_str += ")";
            return Type::tuple(type_str);
        }

        // Tuple index access: tuple.0, tuple.1, etc.
        if (auto ti = dynamic_cast<AstTupleIndexExpr*>(e)) {
            Type tuple_type = visit_expr(ti->tuple.get());
            if (tuple_type.kind != TypeKind::Tuple) {
                diag_.error("tuple index on non-tuple type " + tuple_type.name(), e->line, e->column);
                return Type::unknown();
            }
            // Parse the tuple type to get element types
            // Format: "(T1, T2, T3)"
            std::string inner = tuple_type.struct_name.substr(1, tuple_type.struct_name.size() - 2);
            std::vector<std::string> elem_types;
            size_t start = 0;
            int depth = 0;
            for (size_t i = 0; i <= inner.size(); ++i) {
                char c = (i < inner.size()) ? inner[i] : ',';
                if (c == '(' || c == '<') depth++;
                else if (c == ')' || c == '>') depth--;
                else if ((c == ',' || i == inner.size()) && depth == 0) {
                    std::string elem = inner.substr(start, i - start);
                    // Trim whitespace
                    while (!elem.empty() && elem.front() == ' ') elem.erase(0, 1);
                    while (!elem.empty() && elem.back() == ' ') elem.pop_back();
                    if (!elem.empty()) elem_types.push_back(elem);
                    start = i + 1;
                }
            }
            if (ti->index < 0 || ti->index >= (int)elem_types.size()) {
                diag_.error("tuple index out of bounds: index " + std::to_string(ti->index) + " on tuple with " + std::to_string(elem_types.size()) + " elements", e->line, e->column);
                return Type::unknown();
            }
            return parse_type_name(elem_types[ti->index]);
        }

        if (auto s = dynamic_cast<AstStructLiteralExpr*>(e)) {
            // Extract base struct name (strip generic args like "Box<i32>" -> "Box")
            std::string base_type_name = s->type_name;
            size_t angle = base_type_name.find('<');
            if (angle != std::string::npos) {
                base_type_name = base_type_name.substr(0, angle);
            }

            // Allow built-in generic types (Vec, Result, Option)
            if (base_type_name == "Vec" || base_type_name == "Result" || base_type_name == "Option") {
                // Visit all field expressions
                for (auto& field_init : s->fields) {
                    visit_expr(field_init.value.get());
                }
                Type t = Type::unknown();
                t.struct_name = s->type_name;
                return t;
            }

            // Check if the struct type exists
            auto it = struct_types_.find(base_type_name);
            if (it == struct_types_.end()) {
                std::string suggestion = find_similar_name(s->type_name);
                std::string msg = "unknown struct type '" + s->type_name + "'";
                if (!suggestion.empty()) msg += "; did you mean '" + suggestion + "'?";
                diag_.error(msg, e->line, e->column);
                return Type::unknown();
            }

            auto* struct_decl = it->second;
            
            // Check visibility - structs from other modules must be public
            auto* struct_sym = lookup(base_type_name);
            if (struct_sym) {
                check_visibility(struct_sym, e->line, e->column);
            }

            if (s->is_named) {
                // Named field initialization - check field names and types
                for (auto& field_init : s->fields) {
                    bool found = false;
                    for (auto& field : struct_decl->fields) {
                        if (field.name == field_init.field_name) {
                            found = true;
                            Type expected = parse_type_name(field.type_name);
                            Type actual = visit_expr(field_init.value.get());
                            // Skip type check if either type is unknown (e.g., generic type param)
                            if (actual != expected && actual.kind != TypeKind::Unknown && expected.kind != TypeKind::Unknown) {
                                diag_.error("type mismatch in struct field initialization",
                                    field_init.line, field_init.column);
                            }
                            break;
                        }
                    }
                    if (!found) {
                        diag_.error("unknown struct field", field_init.line, field_init.column);
                    }
                }
            } else {
                // Positional initialization - check field count and types
                if (s->fields.size() > struct_decl->fields.size()) {
                    diag_.error("too many initializers for struct", e->line, e->column);
                }
                for (size_t i = 0; i < s->fields.size() && i < struct_decl->fields.size(); ++i) {
                    Type expected = parse_type_name(struct_decl->fields[i].type_name);
                    Type actual = visit_expr(s->fields[i].value.get());
                    // Skip type check if either type is unknown (e.g., generic type param)
                    if (actual != expected && actual.kind != TypeKind::Unknown && expected.kind != TypeKind::Unknown) {
                        diag_.error("type mismatch in struct field initialization",
                            s->fields[i].line, s->fields[i].column);
                    }
                }
            }

            return Type::struct_(s->type_name);
        }

        if (auto s = dynamic_cast<AstScopeAccessExpr*>(e)) {
            // Check if it's an enum type
            auto it = enum_types_.find(s->scope_name);
            if (it != enum_types_.end()) {
                // Check visibility - enums from other modules must be public
                auto* enum_sym = lookup(s->scope_name);
                if (enum_sym) {
                    check_visibility(enum_sym, e->line, e->column);
                }
                
                // Check if the variant exists
                bool found = false;
                for (auto& variant : it->second->variants) {
                    if (variant.name == s->member_name) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    diag_.error("unknown enum variant '" + s->member_name + "' for enum '" + s->scope_name + "'", e->line, e->column);
                }
                return Type::enum_(s->scope_name);
            }
            // Check if it's a struct constant (Type::CONST)
            std::string const_lookup = s->scope_name + "_" + s->member_name;
            auto* const_sym = lookup(const_lookup);
            if (const_sym) {
                return const_sym->type;
            }
            // Could be a module scope access - for now, return unknown
            diag_.error("unknown scope", e->line, e->column);
            return Type::unknown();
        }

        if (dynamic_cast<AstSelfExpr*>(e)) {
            if (current_receiver_type_.kind == TypeKind::Unknown) {
                diag_.error("'self' used outside of method", e->line, e->column);
                return Type::unknown();
            }
            return current_receiver_type_;
        }

        if (dynamic_cast<AstNoneExpr*>(e)) {
            // None is compatible with any Option<T> - return a special type
            // The type checker will allow None in contexts expecting Option<T>
            Type t;
            t.kind = TypeKind::Struct;
            t.struct_name = "None";  // Special marker type
            return t;
        }

        if (auto m = dynamic_cast<AstMatchExpr*>(e)) {
            // Visit the value being matched and get its type
            Type match_value_type = visit_expr(m->value.get());

            // Visit each arm and track result type
            Type result_type = Type::unknown();
            for (auto& arm : m->arms) {
                bool created_scope = false;
                
                // Handle binding pattern (e.g., 'n if n < 5 => ...')
                if (arm.has_binding()) {
                    push_scope();
                    created_scope = true;
                    declare(arm.binding, { arm.binding, match_value_type, true });
                }
                
                // Visit pattern (for type checking enum variants, etc.)
                // Skip wildcard patterns (_)
                if (!arm.patterns.empty()) {
                    if (auto* optPat = dynamic_cast<AstOptionPattern*>(arm.patterns[0].get())) {
                        // Option/Result pattern - create binding scope if there's a binding
                        if (!optPat->binding.empty()) {
                            push_scope();
                            created_scope = true;
                            
                            // Determine the inner type based on pattern kind
                            Type inner_type = Type::unknown();
                            std::string type_name = match_value_type.struct_name;
                            
                            if (optPat->pattern_kind == "Some") {
                                // Extract T from Option<T>
                                if (type_name.rfind("Option<", 0) == 0) {
                                    size_t end = type_name.rfind('>');
                                    if (end != std::string::npos) {
                                        inner_type = parse_type_name(type_name.substr(7, end - 7));
                                    }
                                }
                            } else if (optPat->pattern_kind == "Ok") {
                                // Extract T from Result<T, E>
                                if (type_name.rfind("Result<", 0) == 0) {
                                    size_t comma = type_name.find(',');
                                    if (comma != std::string::npos) {
                                        inner_type = parse_type_name(type_name.substr(7, comma - 7));
                                    }
                                }
                            } else if (optPat->pattern_kind == "Err") {
                                // Extract E from Result<T, E>
                                if (type_name.rfind("Result<", 0) == 0) {
                                    size_t comma = type_name.find(',');
                                    size_t end = type_name.rfind('>');
                                    if (comma != std::string::npos && end != std::string::npos) {
                                        std::string err_type = type_name.substr(comma + 1, end - comma - 1);
                                        // Trim whitespace
                                        size_t start = err_type.find_first_not_of(' ');
                                        if (start != std::string::npos) {
                                            err_type = err_type.substr(start);
                                        }
                                        inner_type = parse_type_name(err_type);
                                    }
                                }
                            }
                            
                            declare(optPat->binding, { optPat->binding, inner_type, true });
                        }
                        // Don't visit pattern as expression (it's not a runtime expression)
                    } else if (auto* enumPat = dynamic_cast<AstEnumPattern*>(arm.patterns[0].get())) {
                        // Enum variant destructuring pattern: Enum::Variant(x, y) or Enum::Variant { f: x }
                        if (enum_types_.count(enumPat->enum_name)) {
                            auto* ed = enum_types_[enumPat->enum_name];
                            // Find the variant
                            for (const auto& v : ed->variants) {
                                if (v.name == enumPat->variant_name) {
                                    if (!enumPat->bindings.empty() || !enumPat->field_bindings.empty()) {
                                        push_scope();
                                        created_scope = true;
                                        
                                        // Bind tuple fields
                                        if (enumPat->is_tuple_pattern && v.is_tuple_variant()) {
                                            for (size_t i = 0; i < enumPat->bindings.size() && i < v.tuple_types.size(); ++i) {
                                                if (enumPat->bindings[i] != "_") {
                                                    Type field_type = parse_type_name(v.tuple_types[i]);
                                                    declare(enumPat->bindings[i], { enumPat->bindings[i], field_type, true });
                                                }
                                            }
                                        }
                                        // Bind struct fields
                                        else if (!enumPat->is_tuple_pattern && v.is_struct_variant()) {
                                            for (const auto& fb : enumPat->field_bindings) {
                                                for (const auto& sf : v.struct_fields) {
                                                    if (sf.name == fb.first) {
                                                        Type field_type = parse_type_name(sf.type_name);
                                                        declare(fb.second, { fb.second, field_type, true });
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        // Don't visit pattern as expression
                    } else if (auto* id = dynamic_cast<AstIdentifierExpr*>(arm.patterns[0].get())) {
                        if (id->name == "_") {
                            // Wildcard - skip type checking
                        } else if (!id->name.empty() && std::islower(id->name[0])) {
                            // Binding pattern (lowercase identifier) - create scope and declare binding
                            push_scope();
                            created_scope = true;
                            declare(id->name, { id->name, match_value_type, true });
                        } else {
                            // Constant pattern (uppercase or known constant) - visit as expression
                            visit_expr(arm.patterns[0].get());
                        }
                    } else {
                        visit_expr(arm.patterns[0].get());
                    }
                }

                // Visit guard expression if present (after binding is in scope)
                if (arm.guard) {
                    visit_expr(arm.guard.get());
                }

                // Visit result expression
                Type arm_type = visit_expr(arm.result.get());
                
                if (created_scope) {
                    pop_scope();
                }

                // Check that all arms have the same type
                if (result_type.kind == TypeKind::Unknown) {
                    result_type = arm_type;
                } else if (arm_type != result_type && arm_type.kind != TypeKind::Unknown) {
                    diag_.error("match arms have different types", arm.line, arm.column);
                }
            }

            // Check for exhaustive match on enum types
            if ((match_value_type.kind == TypeKind::Struct || match_value_type.kind == TypeKind::Enum) && enum_types_.count(match_value_type.struct_name)) {
                auto* ed = enum_types_[match_value_type.struct_name];
                std::set<std::string> covered_variants;
                bool has_wildcard = false;

                for (const auto& arm : m->arms) {
                    for (const auto& pat : arm.patterns) {
                        if (auto* id = dynamic_cast<AstIdentifierExpr*>(pat.get())) {
                            if (id->name == "_") {
                                has_wildcard = true;
                            } else if (id->name.find("::") != std::string::npos) {
                                // EnumName::Variant pattern
                                size_t pos = id->name.rfind("::");
                                std::string variant = id->name.substr(pos + 2);
                                covered_variants.insert(variant);
                            }
                        } else if (auto* enumPat = dynamic_cast<AstEnumPattern*>(pat.get())) {
                            covered_variants.insert(enumPat->variant_name);
                        } else if (auto* memAcc = dynamic_cast<AstMemberAccessExpr*>(pat.get())) {
                            // Handle Enum::Variant as MemberAccess
                            covered_variants.insert(memAcc->member_name);
                        }
                    }
                }

                if (!has_wildcard) {
                    std::vector<std::string> missing;
                    for (const auto& v : ed->variants) {
                        if (covered_variants.find(v.name) == covered_variants.end()) {
                            missing.push_back(v.name);
                        }
                    }
                    if (!missing.empty()) {
                        std::string msg = "non-exhaustive match: missing variants ";
                        for (size_t i = 0; i < missing.size(); ++i) {
                            if (i > 0) msg += ", ";
                            msg += ed->name + "::" + missing[i];
                        }
                        diag_.warning(msg, m->line, m->column);
                    }
                }
            }

            return result_type;
        }

        if (auto c = dynamic_cast<AstClosureExpr*>(e)) {
            push_scope();

            // Register closure parameters and build parameter type string
            std::string param_types;
            for (size_t i = 0; i < c->params.size(); ++i) {
                auto& param = c->params[i];
                Type param_type = param.type_name.empty()
                    ? Type::unknown()  // Type will be inferred
                    : parse_type_name(param.type_name);
                declare(param.name, { param.name, param_type, true });
                
                if (i > 0) param_types += ", ";
                param_types += param_type.name();
            }

            // Visit body and determine return type
            Type body_type;
            if (c->body_block) {
                // For block closures, analyze return statements
                visit_stmt(c->body_block.get());
                // Check if there's an explicit return type annotation
                if (!c->return_type.empty()) {
                    body_type = parse_type_name(c->return_type);
                } else {
                    body_type = Type::void_();  // Default to void for block closures
                }
            } else {
                body_type = visit_expr(c->body_expr.get());
            }

            pop_scope();

            // Return a proper function type with inferred return type
            return Type::function(param_types, body_type.name());
        }

        if (auto t = dynamic_cast<AstTryExpr*>(e)) {
            // Try expression: expr? unwraps Result/Option or propagates error
            Type operand_type = visit_expr(t->operand.get());
            
            // Extract inner type from Result<T, E> or Option<T>
            std::string type_name = operand_type.name();
            
            // Handle Result<T, E> - extract T
            if (type_name.rfind("Result<", 0) == 0) {
                size_t start = 7;  // After "Result<"
                size_t comma = type_name.find(',', start);
                size_t end = (comma != std::string::npos) ? comma : type_name.find('>', start);
                if (end != std::string::npos) {
                    std::string inner = type_name.substr(start, end - start);
                    // Trim whitespace
                    size_t first = inner.find_first_not_of(' ');
                    size_t last = inner.find_last_not_of(' ');
                    if (first != std::string::npos) {
                        inner = inner.substr(first, last - first + 1);
                    }
                    return parse_type_name(inner);
                }
            }
            
            // Handle Option<T> - extract T
            if (type_name.rfind("Option<", 0) == 0) {
                size_t start = 7;  // After "Option<"
                size_t end = type_name.rfind('>');
                if (end != std::string::npos && end > start) {
                    std::string inner = type_name.substr(start, end - start);
                    // Trim whitespace
                    size_t first = inner.find_first_not_of(' ');
                    size_t last = inner.find_last_not_of(' ');
                    if (first != std::string::npos) {
                        inner = inner.substr(first, last - first + 1);
                    }
                    return parse_type_name(inner);
                }
            }
            
            // Fallback for unknown wrapper types
            return Type::unknown();
        }

        if (auto oc = dynamic_cast<AstOptionalChainExpr*>(e)) {
            // Optional chaining: expr?.field or expr?.method()
            // Visit the object to check it
            visit_expr(oc->object.get());
            
            // Visit method arguments if this is a method call
            if (oc->is_method_call) {
                for (auto& arg : oc->args) {
                    visit_expr(dynamic_cast<AstExpr*>(arg.get()));
                }
            }
            
            // The result is always Option<T> where T is the field/method return type
            // For now, return unknown
            return Type::unknown();
        }

        if (auto nc = dynamic_cast<AstNullCoalesceExpr*>(e)) {
            // Null coalescing: opt ?? default
            // Visit both expressions
            visit_expr(nc->option_expr.get());
            Type default_type = visit_expr(nc->default_expr.get());
            // Result type is the inner type of Option (or the default type)
            return default_type;
        }

        if (auto f = dynamic_cast<AstFStringExpr*>(e)) {
            // Visit each expression part in the f-string
            for (auto& part : f->parts) {
                if (part.is_expr && part.expr) {
                    visit_expr(part.expr.get());
                }
            }
            return Type::string();
        }

        if (auto o = dynamic_cast<AstOrExpr*>(e)) {
            // Or expression: expr or return/break/{ block }
            // LHS must be Result<T, E>, result is unwrapped T
            Type lhs_type = visit_expr(o->lhs.get());
            std::string type_name = lhs_type.name();

            // Check that LHS is Result<T, E>
            if (type_name.rfind("Result<", 0) != 0) {
                diag_.error("'or' operator requires Result type, got '" + type_name + "'",
                           o->line, o->column);
                return Type::unknown();
            }

            // Visit the fallback to check it's valid
            if (o->has_block()) {
                visit_stmt(o->fallback_block.get());
                // Check that block diverges (ends with return/break/continue/panic)
                if (!always_returns(o->fallback_block.get())) {
                    diag_.error("'or' block must not fall through (must return, break, or continue)",
                               o->line, o->column);
                }
            } else if (o->fallback_stmt) {
                visit_stmt(o->fallback_stmt.get());
                // return/break/continue are inherently diverging
            }

            // Extract T from Result<T, E>
            size_t start = 7;  // After "Result<"
            size_t comma = type_name.find(',', start);
            size_t end = (comma != std::string::npos) ? comma : type_name.find('>', start);
            if (end != std::string::npos) {
                std::string inner = type_name.substr(start, end - start);
                // Trim whitespace
                size_t first = inner.find_first_not_of(' ');
                size_t last = inner.find_last_not_of(' ');
                if (first != std::string::npos) {
                    inner = inner.substr(first, last - first + 1);
                }
                return parse_type_name(inner);
            }

            return Type::unknown();
        }

        return Type::unknown();
    }


    // ========== "Did you mean?" suggestions ==========

    size_t SemanticAnalyzer::levenshtein_distance(const std::string& a, const std::string& b) {
        const size_t m = a.size();
        const size_t n = b.size();
        if (m == 0) return n;
        if (n == 0) return m;

        std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
        for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
        for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

        for (size_t i = 1; i <= m; ++i) {
            for (size_t j = 1; j <= n; ++j) {
                size_t cost = (a[i-1] == b[j-1]) ? 0 : 1;
                dp[i][j] = std::min({
                    dp[i-1][j] + 1,      // deletion
                    dp[i][j-1] + 1,      // insertion
                    dp[i-1][j-1] + cost  // substitution
                });
            }
        }
        return dp[m][n];
    }

    std::vector<std::string> SemanticAnalyzer::get_all_known_names() {
        std::vector<std::string> names;
        // Add all variables from all scopes
        for (const auto& scope : scopes_) {
            for (const auto& [name, _] : scope) {
                names.push_back(name);
            }
        }
        // Add builtin functions
        for (const auto& [name, _] : builtin_functions_) {
            names.push_back(name);
        }
        // Add struct types
        for (const auto& [name, _] : struct_types_) {
            names.push_back(name);
        }
        // Add enum types
        for (const auto& [name, _] : enum_types_) {
            names.push_back(name);
        }
        // Add function declarations
        for (const auto& [name, _] : func_decls_) {
            names.push_back(name);
        }
        return names;
    }

    std::string SemanticAnalyzer::find_similar_name(const std::string& name, int max_distance) {
        std::string best_match;
        size_t best_distance = static_cast<size_t>(max_distance) + 1;

        for (const auto& known : get_all_known_names()) {
            // Skip if length difference is too large
            if (std::abs(static_cast<int>(known.size()) - static_cast<int>(name.size())) > max_distance) {
                continue;
            }
            size_t dist = levenshtein_distance(name, known);
            if (dist < best_distance && dist <= static_cast<size_t>(max_distance)) {
                best_distance = dist;
                best_match = known;
            }
        }
        return best_match;
    }

    // ========== Unused variable tracking ==========

    void SemanticAnalyzer::mark_variable_used(const std::string& name) {
        variable_used_[name] = true;
    }

    void SemanticAnalyzer::check_unused_variables() {
        for (const auto& [name, used] : variable_used_) {
            if (!used && name[0] != '_') {  // Skip underscore-prefixed variables
                auto loc = variable_location_.find(name);
                if (loc != variable_location_.end()) {
                    diag_.warning("unused variable '" + name + "' (prefix with '_' to silence)", 
                                  loc->second.first, loc->second.second);
                }
            }
        }
    }



    // ========== Constant Folding ==========

    bool SemanticAnalyzer::try_fold_constant(AstExpr* e, ConstValue& result) {
        // Try to evaluate a literal
        if (auto* lit = dynamic_cast<AstLiteralExpr*>(e)) {
            if (lit->is_string) {
                result.kind = ConstValue::String;
                result.string_val = lit->value;
                return true;
            }
            if (lit->value == "true") {
                result.kind = ConstValue::Bool;
                result.bool_val = true;
                return true;
            }
            if (lit->value == "false") {
                result.kind = ConstValue::Bool;
                result.bool_val = false;
                return true;
            }
            // Check for float
            if (lit->value.find('.') != std::string::npos ||
                lit->value.find('e') != std::string::npos ||
                lit->value.find('E') != std::string::npos) {
                try {
                    result.kind = ConstValue::Float;
                    result.float_val = std::stod(lit->value);
                    return true;
                } catch (...) {
                    return false;
                }
            }
            // Integer (handle hex, binary, octal)
            try {
                result.kind = ConstValue::Int;
                if (lit->value.size() > 2) {
                    if (lit->value[0] == '0' && (lit->value[1] == 'x' || lit->value[1] == 'X')) {
                        result.int_val = std::stoll(lit->value, nullptr, 16);
                    } else if (lit->value[0] == '0' && (lit->value[1] == 'b' || lit->value[1] == 'B')) {
                        result.int_val = std::stoll(lit->value.substr(2), nullptr, 2);
                    } else if (lit->value[0] == '0' && (lit->value[1] == 'o' || lit->value[1] == 'O')) {
                        result.int_val = std::stoll(lit->value.substr(2), nullptr, 8);
                    } else {
                        result.int_val = std::stoll(lit->value);
                    }
                } else {
                    result.int_val = std::stoll(lit->value);
                }
                return true;
            } catch (...) {
                return false;
            }
        }

        // Try to fold binary expression
        if (auto* bin = dynamic_cast<AstBinaryExpr*>(e)) {
            ConstValue left_val, right_val;
            if (!try_fold_constant(static_cast<AstExpr*>(bin->left.get()), left_val)) return false;
            if (!try_fold_constant(static_cast<AstExpr*>(bin->right.get()), right_val)) return false;

            // Integer operations
            if (left_val.kind == ConstValue::Int && right_val.kind == ConstValue::Int) {
                result.kind = ConstValue::Int;
                if (bin->op == "+") { result.int_val = left_val.int_val + right_val.int_val; return true; }
                if (bin->op == "-") { result.int_val = left_val.int_val - right_val.int_val; return true; }
                if (bin->op == "*") { result.int_val = left_val.int_val * right_val.int_val; return true; }
                if (bin->op == "/" && right_val.int_val != 0) { result.int_val = left_val.int_val / right_val.int_val; return true; }
                if (bin->op == "%" && right_val.int_val != 0) { result.int_val = left_val.int_val % right_val.int_val; return true; }
                if (bin->op == "**") {
                    result.int_val = 1;
                    for (int64_t i = 0; i < right_val.int_val; ++i) result.int_val *= left_val.int_val;
                    return true;
                }
                // Comparison operators return bool
                result.kind = ConstValue::Bool;
                if (bin->op == "==") { result.bool_val = left_val.int_val == right_val.int_val; return true; }
                if (bin->op == "!=") { result.bool_val = left_val.int_val != right_val.int_val; return true; }
                if (bin->op == "<") { result.bool_val = left_val.int_val < right_val.int_val; return true; }
                if (bin->op == "<=") { result.bool_val = left_val.int_val <= right_val.int_val; return true; }
                if (bin->op == ">") { result.bool_val = left_val.int_val > right_val.int_val; return true; }
                if (bin->op == ">=") { result.bool_val = left_val.int_val >= right_val.int_val; return true; }
                return false;
            }

            // Float operations (promote int to float if needed)
            if ((left_val.kind == ConstValue::Float || left_val.kind == ConstValue::Int) &&
                (right_val.kind == ConstValue::Float || right_val.kind == ConstValue::Int)) {
                double lv = left_val.kind == ConstValue::Float ? left_val.float_val : static_cast<double>(left_val.int_val);
                double rv = right_val.kind == ConstValue::Float ? right_val.float_val : static_cast<double>(right_val.int_val);
                result.kind = ConstValue::Float;
                if (bin->op == "+") { result.float_val = lv + rv; return true; }
                if (bin->op == "-") { result.float_val = lv - rv; return true; }
                if (bin->op == "*") { result.float_val = lv * rv; return true; }
                if (bin->op == "/" && rv != 0.0) { result.float_val = lv / rv; return true; }
                // Comparison operators return bool
                result.kind = ConstValue::Bool;
                if (bin->op == "==") { result.bool_val = lv == rv; return true; }
                if (bin->op == "!=") { result.bool_val = lv != rv; return true; }
                if (bin->op == "<") { result.bool_val = lv < rv; return true; }
                if (bin->op == "<=") { result.bool_val = lv <= rv; return true; }
                if (bin->op == ">") { result.bool_val = lv > rv; return true; }
                if (bin->op == ">=") { result.bool_val = lv >= rv; return true; }
            }

            // Boolean operations
            if (left_val.kind == ConstValue::Bool && right_val.kind == ConstValue::Bool) {
                result.kind = ConstValue::Bool;
                if (bin->op == "&&") { result.bool_val = left_val.bool_val && right_val.bool_val; return true; }
                if (bin->op == "||") { result.bool_val = left_val.bool_val || right_val.bool_val; return true; }
                if (bin->op == "==") { result.bool_val = left_val.bool_val == right_val.bool_val; return true; }
                if (bin->op == "!=") { result.bool_val = left_val.bool_val != right_val.bool_val; return true; }
            }

            // String concatenation
            if (left_val.kind == ConstValue::String && right_val.kind == ConstValue::String) {
                if (bin->op == "+") {
                    result.kind = ConstValue::String;
                    result.string_val = left_val.string_val + right_val.string_val;
                    return true;
                }
            }
        }

        // Try to fold unary expression
        if (auto* un = dynamic_cast<AstUnaryExpr*>(e)) {
            ConstValue operand_val;
            if (!try_fold_constant(static_cast<AstExpr*>(un->right.get()), operand_val)) return false;

            if (un->op == "-") {
                if (operand_val.kind == ConstValue::Int) {
                    result.kind = ConstValue::Int;
                    result.int_val = -operand_val.int_val;
                    return true;
                }
                if (operand_val.kind == ConstValue::Float) {
                    result.kind = ConstValue::Float;
                    result.float_val = -operand_val.float_val;
                    return true;
                }
            }
            if (un->op == "!") {
                if (operand_val.kind == ConstValue::Bool) {
                    result.kind = ConstValue::Bool;
                    result.bool_val = !operand_val.bool_val;
                    return true;
                }
            }
        }

        return false;
    }

    void SemanticAnalyzer::fold_constants_in_expr(std::unique_ptr<AstNode>& e) {
        if (!e) return;

        // Recursively fold sub-expressions first
        if (auto* bin = dynamic_cast<AstBinaryExpr*>(e.get())) {
            fold_constants_in_expr(bin->left);
            fold_constants_in_expr(bin->right);
        } else if (auto* un = dynamic_cast<AstUnaryExpr*>(e.get())) {
            fold_constants_in_expr(un->right);
        }

        // Try to fold this expression
        auto* expr = dynamic_cast<AstExpr*>(e.get());
        if (!expr) return;

        ConstValue result;
        if (try_fold_constant(expr, result)) {
            // Replace with literal
            int line = expr->line;
            int col = expr->column;

            switch (result.kind) {
                case ConstValue::Int:
                    e = std::make_unique<AstLiteralExpr>(std::to_string(result.int_val), false, false, line, col);
                    break;
                case ConstValue::Float: {
                    std::ostringstream oss;
                    oss << std::setprecision(17) << result.float_val;
                    std::string val = oss.str();
                    if (val.find('.') == std::string::npos && val.find('e') == std::string::npos) {
                        val += ".0";
                    }
                    e = std::make_unique<AstLiteralExpr>(val, false, false, line, col);
                    break;
                }
                case ConstValue::Bool:
                    e = std::make_unique<AstLiteralExpr>(result.bool_val ? "true" : "false", false, false, line, col);
                    break;
                case ConstValue::String:
                    e = std::make_unique<AstLiteralExpr>(result.string_val, true, false, line, col);
                    break;
                default:
                    break;
            }
        }
    }

    void SemanticAnalyzer::fold_constants_in_stmt(AstStmt* s) {
        if (auto* block = dynamic_cast<AstBlockStmt*>(s)) {
            for (auto& st : block->statements) {
                fold_constants_in_stmt(st.get());
            }
        } else if (auto* var = dynamic_cast<AstVarDeclStmt*>(s)) {
            if (var->init_expr) {
                fold_constants_in_expr(var->init_expr);
            }
        } else if (auto* ret = dynamic_cast<AstReturnStmt*>(s)) {
            if (ret->value) {
                fold_constants_in_expr(ret->value);
            }
        } else if (auto* expr_stmt = dynamic_cast<AstExprStmt*>(s)) {
            std::unique_ptr<AstNode> temp = std::move(expr_stmt->expr);
            fold_constants_in_expr(temp);
            expr_stmt->expr = std::move(temp);
        } else if (auto* if_stmt = dynamic_cast<AstIfStmt*>(s)) {
            if (if_stmt->condition) {
                std::unique_ptr<AstNode> temp = std::move(if_stmt->condition);
                fold_constants_in_expr(temp);
                if_stmt->condition = std::unique_ptr<AstExpr>(static_cast<AstExpr*>(temp.release()));
            }
            if (if_stmt->then_block) fold_constants_in_stmt(if_stmt->then_block.get());
            if (if_stmt->else_block) fold_constants_in_stmt(if_stmt->else_block.get());
        } else if (auto* while_stmt = dynamic_cast<AstWhileStmt*>(s)) {
            if (while_stmt->condition) {
                std::unique_ptr<AstNode> temp = std::move(while_stmt->condition);
                fold_constants_in_expr(temp);
                while_stmt->condition = std::unique_ptr<AstExpr>(static_cast<AstExpr*>(temp.release()));
            }
            if (while_stmt->body) fold_constants_in_stmt(while_stmt->body.get());
        } else if (auto* for_stmt = dynamic_cast<AstForInStmt*>(s)) {
            if (for_stmt->body) fold_constants_in_stmt(for_stmt->body.get());
        }
    }

    void SemanticAnalyzer::fold_constants_in_module(AstModule* module) {
        for (auto& decl : module->decls) {
            if (auto* fn = dynamic_cast<AstFuncDecl*>(decl.get())) {
                if (fn->body) {
                    fold_constants_in_stmt(fn->body.get());
                }
            } else if (auto* impl = dynamic_cast<AstImplDecl*>(decl.get())) {
                for (auto& method : impl->methods) {
                    if (method->body) {
                        fold_constants_in_stmt(method->body.get());
                    }
                }
            }
        }
    }

} // namespace mana::frontend
