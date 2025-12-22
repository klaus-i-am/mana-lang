#include "Parser.h"

namespace mana::frontend {

    Parser::Parser(const std::vector<Token>& tokens, DiagnosticEngine& diag)
        : tokens_(tokens), diag_(diag) {
    }

    const Token& Parser::peek() const { return tokens_[current_]; }
    const Token& Parser::previous() const { return tokens_[current_ - 1]; }
    bool Parser::is_at_end() const { return peek().kind == TokenKind::EndOfFile; }

    bool Parser::check(TokenKind kind) const {
        if (is_at_end()) return false;
        return peek().kind == kind;
    }

    bool Parser::check_next(TokenKind kind) const {
        if (current_ + 1 >= tokens_.size()) return false;
        return tokens_[current_ + 1].kind == kind;
    }

    const Token& Parser::advance() {
        if (!is_at_end()) current_++;
        return previous();
    }

    bool Parser::match(TokenKind kind) {
        if (check(kind)) { advance(); return true; }
        return false;
    }

    bool Parser::expect(TokenKind kind, const char* msg) {
        if (check(kind)) { advance(); return true; }
        diag_.error(msg, peek().line, peek().column);
        return false;
    }

    void Parser::optional_semicolon() {
        // Consume semicolon if present, but don't error if missing (vNext: semicolons optional)
        match(TokenKind::Semicolon);
    }

    void Parser::synchronize() {
        advance();
        while (!is_at_end()) {
            // After a semicolon, we're at a statement boundary
            if (previous().kind == TokenKind::Semicolon) return;
            // After a closing brace, we might be at a declaration boundary
            if (previous().kind == TokenKind::RBrace) return;

            // These tokens typically start statements or declarations
            switch (peek().kind) {
            case TokenKind::KwFn:
            case TokenKind::KwStruct:
            case TokenKind::KwLet:
            case TokenKind::KwIf:
            case TokenKind::KwWhile:
            case TokenKind::KwFor:
            case TokenKind::KwReturn:
            case TokenKind::KwBreak:
            case TokenKind::KwContinue:
            case TokenKind::KwDefer:
                return;
            default:
                break;
            }
            advance();
        }
    }
    // Synchronize to next statement boundary within a block
    void Parser::synchronize_statement() {
        while (!is_at_end() && !check(TokenKind::RBrace)) {
            // If we're at a semicolon, consume it and we're done
            if (previous().kind == TokenKind::Semicolon) return;
            if (match(TokenKind::Semicolon)) return;

            // These tokens start new statements
            switch (peek().kind) {
            case TokenKind::KwLet:
            case TokenKind::KwIf:
            case TokenKind::KwWhile:
            case TokenKind::KwFor:
            case TokenKind::KwReturn:
            case TokenKind::KwBreak:
            case TokenKind::KwContinue:
            case TokenKind::KwDefer:
            case TokenKind::KwLoop:
            case TokenKind::KwScope:
            case TokenKind::KwConst:
            case TokenKind::KwMatch:
                return;
            default:
                advance();
            }
        }
    }


    std::unique_ptr<AstModule> Parser::parse_module() {
        expect(TokenKind::KwModule, "expected 'module'");
        expect(TokenKind::Identifier, "expected module name");
        Token name = previous();
        optional_semicolon();  // vNext: semicolons optional

        auto mod = std::make_unique<AstModule>(name.lexeme, name.line, name.column);

        while (!is_at_end()) {
            auto d = parse_declaration();
            if (!d) {
                synchronize();
                continue;
            }
            mod->decls.push_back(std::move(d));
        }

        return mod;
    }

    // ---------------- declarations ----------------

    std::unique_ptr<AstDecl> Parser::parse_declaration() {
        // Collect doc comments before the declaration
        std::string doc_comment;
        while (check(TokenKind::DocComment)) {
            if (!doc_comment.empty()) doc_comment += "\n";
            doc_comment += advance().lexeme;
        }

        if (match(TokenKind::KwImport)) return parse_import_decl();

        // Handle #[test] attribute
        bool is_test = false;
        if (match(TokenKind::Hash)) {
            expect(TokenKind::LBracket, "expected '[' after '#'");
            expect(TokenKind::Identifier, "expected attribute name");
            std::string attr_name = previous().lexeme;
            if (attr_name == "test") {
                is_test = true;
            }
            expect(TokenKind::RBracket, "expected ']' after attribute");
        }

        // Handle pub modifier - can apply to use, fn, struct, enum, trait, type
        bool is_pub = match(TokenKind::KwPub);

        if (match(TokenKind::KwUse)) {
            auto decl = parse_use_decl(is_pub);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        
        // Handle extern fn (FFI declaration)
        if (match(TokenKind::KwExtern)) {
            expect(TokenKind::KwFn, "expected 'fn' after 'extern'");
            auto decl = parse_function_decl(is_pub, false, is_test, true);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        
        // Handle async fn
        bool is_async = match(TokenKind::KwAsync);
        if (is_async) {
            expect(TokenKind::KwFn, "expected 'fn' after 'async'");
            auto decl = parse_function_decl(is_pub, true, is_test);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwFn)) {
            auto decl = parse_function_decl(is_pub, false, is_test);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwStruct)) {
            auto decl = parse_struct_decl(is_pub);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwEnum)) {
            auto decl = parse_enum_decl(is_pub, false);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwVariant)) {
            auto decl = parse_enum_decl(is_pub, true);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwTrait)) {
            auto decl = parse_trait_decl(is_pub);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwImpl)) {
            auto decl = parse_impl_decl();
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }
        if (match(TokenKind::KwType)) {
            auto decl = parse_type_alias_decl(is_pub);
            if (decl) decl->doc_comment = doc_comment;
            return decl;
        }

        // allow top-level global var: name : type = expr ;
        if (check(TokenKind::Identifier)) {
            // lookahead for ':'
            if (current_ + 1 < tokens_.size() && tokens_[current_ + 1].kind == TokenKind::Colon) {
                auto decl = parse_global_var_decl();
                if (decl) decl->doc_comment = doc_comment;
                return decl;
            }
        }

        diag_.error("unexpected top-level declaration", peek().line, peek().column);
        return nullptr;
    }

    std::unique_ptr<AstDecl> Parser::parse_struct_decl(bool is_pub) {
        expect(TokenKind::Identifier, "expected struct name");
        Token name = previous();

        auto s = std::make_unique<AstStructDecl>(name.lexeme, name.line, name.column);

        // Parse optional type parameters: struct Foo<T, U> { ... }
        if (match(TokenKind::Less)) {
            do {
                expect(TokenKind::Identifier, "expected type parameter name");
                s->type_params.push_back(previous().lexeme);
            } while (match(TokenKind::Comma));
            expect(TokenKind::Greater, "expected '>' after type parameters");
        }

        expect(TokenKind::LBrace, "expected '{' after struct name");

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            expect(TokenKind::Identifier, "expected field name");
            Token field_name = previous();
            expect(TokenKind::Colon, "expected ':' after field name");
            std::string field_type = parse_type_name();
            
            AstStructField field;
            field.name = field_name.lexeme;
            field.type_name = field_type;
            field.line = field_name.line;
            field.column = field_name.column;
            
            // Parse optional default value: field: type = expr
            if (match(TokenKind::Assign)) {
                field.default_value = parse_expression();
            }

            s->fields.push_back(std::move(field));

            // Accept comma or semicolon as field separator (both optional for vNext)
            if (!match(TokenKind::Comma) && !match(TokenKind::Semicolon)) {
                // Neither found - only ok if we're at the closing brace
                if (!check(TokenKind::RBrace)) {
                    diag_.error("expected ',' or '}' after field", peek().line, peek().column);
                }
            }
        }

        expect(TokenKind::RBrace, "expected '}' after struct fields");
        s->is_pub = is_pub;
        return s;
    }

    std::unique_ptr<AstDecl> Parser::parse_enum_decl(bool is_pub, bool declared_as_variant) {
        expect(TokenKind::Identifier, "expected enum name");
        Token name = previous();

        expect(TokenKind::LBrace, "expected '{' after enum name");

        auto e = std::make_unique<AstEnumDecl>(name.lexeme, name.line, name.column);
        e->declared_as_variant = declared_as_variant;
        int next_value = 0;

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            expect(TokenKind::Identifier, "expected variant name");
            Token variant_name = previous();

            AstEnumVariant variant;
            variant.name = variant_name.lexeme;
            variant.line = variant_name.line;
            variant.column = variant_name.column;
            variant.has_value = false;
            variant.value = next_value;

            // Check for associated data
            if (match(TokenKind::LParen)) {
                // Tuple variant: Variant(Type1, Type2)
                if (!check(TokenKind::RParen)) {
                    do {
                        std::string type_name = parse_type_name();
                        variant.tuple_types.push_back(type_name);
                    } while (match(TokenKind::Comma));
                }
                expect(TokenKind::RParen, "expected ')' after tuple variant types");
            } else if (match(TokenKind::LBrace)) {
                // Struct variant: Variant { field: Type, ... }
                while (!check(TokenKind::RBrace) && !is_at_end()) {
                    expect(TokenKind::Identifier, "expected field name in struct variant");
                    std::string field_name = previous().lexeme;
                    int field_line = previous().line;
                    int field_col = previous().column;
                    expect(TokenKind::Colon, "expected ':' after field name");
                    std::string field_type = parse_type_name();

                    AstStructField field;
                    field.name = field_name;
                    field.type_name = field_type;
                    field.line = field_line;
                    field.column = field_col;
                    variant.struct_fields.push_back(std::move(field));

                    if (!check(TokenKind::RBrace)) {
                        expect(TokenKind::Comma, "expected ',' after struct variant field");
                    } else {
                        match(TokenKind::Comma); // trailing comma
                    }
                }
                expect(TokenKind::RBrace, "expected '}' after struct variant fields");
            } else if (match(TokenKind::Assign)) {
                // Explicit value: Variant = 10
                expect(TokenKind::IntLiteral, "expected integer value for enum variant");
                variant.has_value = true;
                variant.value = std::stoi(previous().lexeme);
                next_value = variant.value;
            }

            e->variants.push_back(std::move(variant));
            next_value++;

            // Comma is optional for last variant
            if (!check(TokenKind::RBrace)) {
                expect(TokenKind::Comma, "expected ',' after enum variant");
            } else {
                match(TokenKind::Comma); // allow trailing comma
            }
        }

        expect(TokenKind::RBrace, "expected '}' after enum variants");
        e->is_pub = is_pub;
        return e;
    }

    std::unique_ptr<AstDecl> Parser::parse_import_decl() {
        int l = peek().line;
        int c = peek().column;

        // Check for string literal (file import): import "path/to/file.mana";
        if (match(TokenKind::StringLiteral)) {
            Token path = previous();
            optional_semicolon();  // vNext: semicolons optional
            auto imp = std::make_unique<AstImportDecl>(path.lexeme, l, c);
            imp->path = path.lexeme;
            imp->is_file_import = true;
            return imp;
        }

        // Module import: import std::io;
        expect(TokenKind::Identifier, "expected import name");
        std::string name = previous().lexeme;

        // Handle qualified imports: std::io::file
        while (match(TokenKind::ColonColon)) {
            expect(TokenKind::Identifier, "expected identifier after '::'");
            name += "::" + previous().lexeme;
        }

        optional_semicolon();  // vNext: semicolons optional
        auto imp = std::make_unique<AstImportDecl>(name, l, c);
        imp->is_file_import = false;
        return imp;
    }

    std::unique_ptr<AstDecl> Parser::parse_use_decl(bool is_pub) {
        int l = peek().line;
        int c = peek().column;

        // Parse module path: std::io
        expect(TokenKind::Identifier, "expected module path");
        std::string path = previous().lexeme;

        while (match(TokenKind::ColonColon)) {
            // Check for glob import: use std::io::*
            if (match(TokenKind::Star)) {
                optional_semicolon();  // vNext: semicolons optional
                auto use_decl = std::make_unique<AstUseDecl>(path, l, c);
                use_decl->is_glob = true;
                use_decl->is_pub = is_pub;
                return use_decl;
            }

            // Check for selective import: use std::io::{File, Read}
            if (match(TokenKind::LBrace)) {
                std::vector<std::string> names;
                do {
                    expect(TokenKind::Identifier, "expected name in use");
                    names.push_back(previous().lexeme);
                } while (match(TokenKind::Comma));
                expect(TokenKind::RBrace, "expected '}' in use");
                optional_semicolon();  // vNext: semicolons optional
                auto use_decl = std::make_unique<AstUseDecl>(path, l, c);
                use_decl->imported_names = std::move(names);
                use_decl->is_pub = is_pub;
                return use_decl;
            }

            expect(TokenKind::Identifier, "expected identifier after '::''");
            path += "::" + previous().lexeme;
        }

        // Check for alias: use std::io as io
        std::string alias;
        if (match(TokenKind::KwAs)) {
            expect(TokenKind::Identifier, "expected alias name");
            alias = previous().lexeme;
        }

        optional_semicolon();  // vNext: semicolons optional
        auto use_decl = std::make_unique<AstUseDecl>(path, l, c);
        use_decl->alias = alias;
        use_decl->is_pub = is_pub;
        return use_decl;
    }

    std::unique_ptr<AstDecl> Parser::parse_trait_decl(bool is_pub) {
        // trait TraitName { fn method(self, args) -> ReturnType; ... }
        expect(TokenKind::Identifier, "expected trait name");
        Token name = previous();

        auto trait = std::make_unique<AstTraitDecl>(name.lexeme, name.line, name.column);

        expect(TokenKind::LBrace, "expected '{' after trait name");

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            // Parse associated type: type Item;
            if (match(TokenKind::KwType)) {
                expect(TokenKind::Identifier, "expected associated type name");
                Token type_name = previous();
                
                AstAssociatedType assoc_type;
                assoc_type.name = type_name.lexeme;
                assoc_type.line = type_name.line;
                assoc_type.column = type_name.column;

                optional_semicolon();  // vNext: semicolons optional
                trait->associated_types.push_back(std::move(assoc_type));
                continue;
            }
            
            expect(TokenKind::KwFn, "expected 'fn' or 'type' in trait body");
            expect(TokenKind::Identifier, "expected method name");
            Token method_name = previous();

            AstTraitMethod method;
            method.name = method_name.lexeme;
            method.line = method_name.line;
            method.column = method_name.column;

            expect(TokenKind::LParen, "expected '('");

            // Parse parameters
            if (!check(TokenKind::RParen)) {
                while (true) {
                    // Check for 'self' parameter
                    if (check(TokenKind::KwSelf)) {
                        advance();
                        method.takes_self = true;
                        if (!check(TokenKind::RParen) && !check(TokenKind::Comma)) {
                            diag_.error("'self' must be first parameter", peek().line, peek().column);
                        }
                    } else {
                        expect(TokenKind::Identifier, "expected parameter name");
                        Token p_name = previous();
                        expect(TokenKind::Colon, "expected ':' after parameter name");
                        std::string p_type = parse_type_name();

                        AstParam p;
                        p.name = p_name.lexeme;
                        p.type_name = p_type;
                        p.line = p_name.line;
                        p.column = p_name.column;
                        method.params.push_back(std::move(p));
                    }

                    if (match(TokenKind::Comma)) continue;
                    break;
                }
            }

            expect(TokenKind::RParen, "expected ')'");
            expect(TokenKind::Arrow, "expected '->'");
            method.return_type = parse_type_name();
            
            // Check for optional default implementation
            if (check(TokenKind::LBrace)) {
                method.body = parse_block();
            } else {
                optional_semicolon();  // vNext: semicolons optional
            }

            trait->methods.push_back(std::move(method));
        }

        expect(TokenKind::RBrace, "expected '}' after trait methods");
        trait->is_pub = is_pub;
        return trait;
    }

    std::unique_ptr<AstDecl> Parser::parse_impl_decl() {
        // impl TraitName for TypeName { methods... }
        // OR impl TypeName { methods... } (inherent impl)
        expect(TokenKind::Identifier, "expected trait or type name");
        Token first_name = previous();
        int l = first_name.line;
        int c = first_name.column;

        auto impl = std::make_unique<AstImplDecl>(l, c);

        // Check if this is "impl Trait for Type" or "impl Type"
        if (check(TokenKind::KwFor)) {
            // This is actually 'for' which we need as a keyword... but we're using KwFor
            // Let me check for identifier "for"
        }

        // Simple heuristic: if we see another identifier after the first, it's "impl Trait for Type"
        // Actually, let's check for literal "for" which is KwFor in our tokens
        if (match(TokenKind::KwFor)) {
            // impl TraitName for TypeName
            impl->trait_name = first_name.lexeme;
            expect(TokenKind::Identifier, "expected type name after 'for'");
            impl->type_name = previous().lexeme;
        } else {
            // impl TypeName (inherent impl - methods directly on type)
            impl->type_name = first_name.lexeme;
        }

        expect(TokenKind::LBrace, "expected '{' after impl");

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            // Parse associated type assignment: type Item = ConcreteType;
            if (match(TokenKind::KwType)) {
                expect(TokenKind::Identifier, "expected associated type name");
                Token type_name = previous();
                
                expect(TokenKind::Assign, "expected '=' after associated type name");
                std::string target_type = parse_type_name();
                optional_semicolon();  // vNext: semicolons optional
                
                AstTypeAssignment assignment;
                assignment.name = type_name.lexeme;
                assignment.target_type = target_type;
                assignment.line = type_name.line;
                assignment.column = type_name.column;
                impl->type_assignments.push_back(std::move(assignment));
                continue;
            }

            // Parse constant declaration: const NAME: Type = value;
            if (match(TokenKind::KwConst)) {
                expect(TokenKind::Identifier, "expected constant name");
                Token const_name = previous();
                
                expect(TokenKind::Colon, "expected ':' after constant name");
                std::string const_type = parse_type_name();
                expect(TokenKind::Assign, "expected '=' after constant type");
                auto init_expr = parse_expression();
                optional_semicolon();  // vNext: semicolons optional
                
                AstImplConst impl_const;
                impl_const.name = const_name.lexeme;
                impl_const.type_name = const_type;
                impl_const.init_expr = std::move(init_expr);
                impl_const.line = const_name.line;
                impl_const.column = const_name.column;
                impl->constants.push_back(std::move(impl_const));
                continue;
            }
            
            // Check for static fn
            bool is_static = match(TokenKind::KwStatic);
            
            expect(TokenKind::KwFn, "expected 'fn', 'const', 'type', or 'static' in impl block");
            // Parse the function declaration
            auto fn = parse_function_decl(false);
            if (fn) {
                // Set the receiver type for the method
                auto* func = dynamic_cast<AstFuncDecl*>(fn.get());
                if (func) {
                    func->receiver_type = impl->type_name;
                    func->is_static = is_static;
                    impl->methods.push_back(std::unique_ptr<AstFuncDecl>(
                        static_cast<AstFuncDecl*>(fn.release())));
                }
            }
        }

        expect(TokenKind::RBrace, "expected '}' after impl methods");
        return impl;
    }

    std::string Parser::parse_type_name() {
        // Pointer type: *T
        if (match(TokenKind::Star)) {
            std::string pointee = parse_type_name();
            return "*" + pointee;
        }

        // Reference type: &T or &mut T
        if (match(TokenKind::And)) {
            // Check for mutable reference: &mut T
            if (match(TokenKind::KwMut)) {
                std::string referent = parse_type_name();
                return "&mut " + referent;
            }
            std::string referent = parse_type_name();
            return "&" + referent;
        }

        // Dynamic trait object: dyn TraitName
        if (match(TokenKind::KwDyn)) {
            expect(TokenKind::Identifier, "expected trait name after 'dyn'");
            return "dyn " + previous().lexeme;
        }

        // Array type: [N]T or []T
        if (match(TokenKind::LBracket)) {
            std::string size_str = "";
            if (check(TokenKind::IntLiteral)) {
                advance();
                size_str = previous().lexeme;
            }
            expect(TokenKind::RBracket, "expected ']' in array type");
            std::string elem_type = parse_type_name();
            return "[" + size_str + "]" + elem_type;
        }

        // Tuple type: (T1, T2, ...)
        if (match(TokenKind::LParen)) {
            std::string result = "(";
            if (!check(TokenKind::RParen)) {
                result += parse_type_name();
                while (match(TokenKind::Comma)) {
                    result += ", ";
                    result += parse_type_name();
                }
            }
            expect(TokenKind::RParen, "expected ')' in tuple type");
            result += ")";
            return result;
        }

        // Handle Self keyword in type position (Self or Self::Item)
        if (match(TokenKind::KwSelf)) {
            std::string type_name = "Self";
            // Check for associated type: Self::Item
            if (match(TokenKind::ColonColon)) {
                expect(TokenKind::Identifier, "expected associated type name after 'Self::'");
                type_name += "::" + previous().lexeme;
            }
            return type_name;
        }

        expect(TokenKind::Identifier, "expected type name");
        std::string type_name = previous().lexeme;

        // Check for path with associated type: TypeName::AssociatedType
        if (match(TokenKind::ColonColon)) {
            expect(TokenKind::Identifier, "expected type name after '::'");
            type_name += "::" + previous().lexeme;
        }

        // Handle generic type arguments: Type<T, U>
        if (match(TokenKind::Less)) {
            type_name += "<";
            type_name += parse_type_name();
            while (match(TokenKind::Comma)) {
                type_name += ", ";
                type_name += parse_type_name();
            }
            expect(TokenKind::Greater, "expected '>' after type arguments");
            type_name += ">";
        }

        return type_name;
    }

    std::unique_ptr<AstDecl> Parser::parse_function_decl(bool is_pub, bool is_async, bool is_test, bool is_extern) {
        expect(TokenKind::Identifier, "expected function name");
        Token first_name = previous();

        std::string receiver_type;
        std::string fn_name_str = first_name.lexeme;
        std::vector<std::string> type_params;

        // Check for method syntax: fn Type.method(...)
        if (match(TokenKind::Dot)) {
            receiver_type = first_name.lexeme;
            expect(TokenKind::Identifier, "expected method name after '.'");
            fn_name_str = previous().lexeme;
        }

        // Parse optional type parameters: fn foo<T, U>(...) or fn Type.method<T>(...)
        if (match(TokenKind::Less)) {
            do {
                expect(TokenKind::Identifier, "expected type parameter name");
                type_params.push_back(previous().lexeme);
            } while (match(TokenKind::Comma));
            expect(TokenKind::Greater, "expected '>' after type parameters");
        }

        expect(TokenKind::LParen, "expected '('");
        std::vector<AstParam> params;
        bool has_self = false;

        if (!check(TokenKind::RParen)) {
            while (true) {
                // Handle 'self' parameter for methods in impl blocks
                if (check(TokenKind::KwSelf)) {
                    advance();
                    has_self = true;
                    // 'self' doesn't need a type - it's implicit
                    if (match(TokenKind::Comma)) continue;
                    break;
                }

                expect(TokenKind::Identifier, "expected parameter name");
                Token p_name = previous();
                expect(TokenKind::Colon, "expected ':' after parameter name");
                std::string p_type = parse_type_name();

                AstParam p;
                p.name = p_name.lexeme;
                p.type_name = p_type;
                p.line = p_name.line;
                p.column = p_name.column;
                
                // Check for default value: = expression
                if (match(TokenKind::Assign)) {
                    p.default_value = parse_expression();
                }
                params.push_back(std::move(p));

                if (match(TokenKind::Comma)) continue;
                break;
            }
        }

        expect(TokenKind::RParen, "expected ')'");

        // Return type: required for most functions, optional for main (defaults to i32)
        std::string ret_type;
        if (match(TokenKind::Arrow)) {
            ret_type = parse_type_name();
        } else if (fn_name_str == "main" && receiver_type.empty()) {
            // main() without explicit return type defaults to i32
            ret_type = "i32";
        } else {
            diag_.error("expected '->' and return type", peek().line, peek().column);
            return nullptr;
        }

        // Parse optional where clause: where T: Trait1 + Trait2, U: Trait3
        std::vector<TypeConstraint> constraints;
        if (match(TokenKind::KwWhere)) {
            do {
                TypeConstraint constraint;
                expect(TokenKind::Identifier, "expected type parameter in where clause");
                constraint.type_param = previous().lexeme;
                constraint.line = previous().line;
                constraint.column = previous().column;
                
                expect(TokenKind::Colon, "expected ':' after type parameter in where clause");
                
                // Parse trait bounds: Trait1 + Trait2 + Trait3
                do {
                    expect(TokenKind::Identifier, "expected trait name in where clause");
                    constraint.traits.push_back(previous().lexeme);
                } while (match(TokenKind::Plus));
                
                constraints.push_back(std::move(constraint));
            } while (match(TokenKind::Comma));
        }

        // Parse body (optional for extern functions)
        std::unique_ptr<AstBlockStmt> body;
        if (is_extern) {
            // Extern functions have no body, just consume optional semicolon
            optional_semicolon();
        } else {
            body = parse_block();
            if (!body) return nullptr;
        }

        auto fn = std::make_unique<AstFuncDecl>(fn_name_str, first_name.line, first_name.column);
        fn->receiver_type = receiver_type;
        fn->type_params = std::move(type_params);
        fn->constraints = std::move(constraints);
        fn->params = std::move(params);
        fn->return_type = ret_type;
        fn->body = std::move(body);
        fn->is_pub = is_pub;
        fn->is_async = is_async;
        fn->is_test = is_test;
        fn->is_extern = is_extern;
        fn->has_self = has_self;
        return fn;
    }

    std::unique_ptr<AstDecl> Parser::parse_global_var_decl() {
        // parse a var decl stmt and wrap as global decl
        auto stmt = parse_var_decl_statement();
        if (!stmt || stmt->kind != NodeKind::VarDeclStmt) return nullptr;

        auto g = std::make_unique<AstGlobalVarDecl>(stmt->line, stmt->column);
        g->var.reset(static_cast<AstVarDeclStmt*>(stmt.release()));
        return g;
    }

    // ---------------- statements ----------------

    std::unique_ptr<AstBlockStmt> Parser::parse_block() {
        expect(TokenKind::LBrace, "expected '{'");
        auto b = std::make_unique<AstBlockStmt>(previous().line, previous().column);

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            auto st = parse_statement();
            if (!st) {
                // Error recovery: skip to next statement boundary
                synchronize_statement();
                continue;
            }
            b->statements.push_back(std::move(st));
        }

        expect(TokenKind::RBrace, "expected '}'");
        return b;
    }

    
    std::unique_ptr<AstDecl> Parser::parse_type_alias_decl(bool is_pub) {
        // type Name = Type;
        Token name_tok = peek();
        expect(TokenKind::Identifier, "expected type alias name");
        std::string alias_name = previous().lexeme;
        
        expect(TokenKind::Assign, "expected '=' after type alias name");
        
        std::string target_type = parse_type_name();

        optional_semicolon();  // vNext: semicolons optional

        auto decl = std::make_unique<AstTypeAliasDecl>(alias_name, target_type, name_tok.line, name_tok.column);
        decl->is_pub = is_pub;
        return decl;
    }

std::unique_ptr<AstStmt> Parser::parse_statement() {
        if (match(TokenKind::KwIf)) return parse_if_statement();
        if (match(TokenKind::KwWhile)) return parse_while_statement();
        if (match(TokenKind::KwLoop)) return parse_loop_statement();
        if (match(TokenKind::KwFor)) return parse_for_statement();
        if (match(TokenKind::KwReturn)) return parse_return_statement();
        if (match(TokenKind::KwBreak)) return parse_break_statement();
        if (match(TokenKind::KwContinue)) return parse_continue_statement();
        if (match(TokenKind::KwDefer)) return parse_defer_statement();
        if (match(TokenKind::KwScope)) return parse_scope_statement();
        if (match(TokenKind::KwLet)) return parse_let_statement();
        if (match(TokenKind::KwConst)) return parse_const_statement();

        // Struct destructuring: {a, b, c}: Type = expr;
        if (check(TokenKind::LBrace)) {
            return parse_destructure_statement(true);
        }

        // Array destructuring: [a, b, c]: [N]Type = expr;
        // Need to look ahead to distinguish from array literal expression
        if (check(TokenKind::LBracket) && (current_ + 1 < tokens_.size()) &&
            tokens_[current_ + 1].kind == TokenKind::Identifier) {
            // Check if followed by comma or closing bracket (destructuring pattern)
            size_t i = current_ + 2;
            while (i < tokens_.size() && tokens_[i].kind != TokenKind::RBracket) {
                i++;
            }
            if (i + 1 < tokens_.size() && tokens_[i + 1].kind == TokenKind::Colon) {
                return parse_destructure_statement(false);
            }
        }

        // var decl: ident ':' ...
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].kind == TokenKind::Colon) {
            return parse_var_decl_statement();
        }

        // assignment statement: ident '=' ...
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].kind == TokenKind::Assign) {
            return parse_assign_statement();
        }

        // increment: ident++
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].kind == TokenKind::PlusPlus) {
            expect(TokenKind::Identifier, "expected identifier");
            Token name = previous();
            advance(); // consume ++
            optional_semicolon();  // vNext: semicolons optional
            // Desugar to: name = name + 1
            auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
            auto one = std::make_unique<AstLiteralExpr>("1", false, false, name.line, name.column);
            auto add = std::make_unique<AstBinaryExpr>("+", name.line, name.column);
            add->left = std::move(id);
            add->right = std::move(one);
            auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
            a->target_name = name.lexeme;
            a->value = std::move(add);
            return a;
        }

        // decrement: ident--
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].kind == TokenKind::MinusMinus) {
            expect(TokenKind::Identifier, "expected identifier");
            Token name = previous();
            advance(); // consume --
            optional_semicolon();  // vNext: semicolons optional
            // Desugar to: name = name - 1
            auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
            auto one = std::make_unique<AstLiteralExpr>("1", false, false, name.line, name.column);
            auto sub = std::make_unique<AstBinaryExpr>("-", name.line, name.column);
            sub->left = std::move(id);
            sub->right = std::move(one);
            auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
            a->target_name = name.lexeme;
            a->value = std::move(sub);
            return a;
        }

        // compound assignment: ident += expr, ident -= expr, etc.
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size())) {
            TokenKind next = tokens_[current_ + 1].kind;
            if (next == TokenKind::PlusEqual || next == TokenKind::MinusEqual ||
                next == TokenKind::StarEqual || next == TokenKind::SlashEqual || next == TokenKind::PercentEqual ||
                next == TokenKind::StarStarEqual ||
                next == TokenKind::AndEqual || next == TokenKind::OrEqual || next == TokenKind::CaretEqual ||
                next == TokenKind::LessLessEqual || next == TokenKind::GreaterGreaterEqual) {
                expect(TokenKind::Identifier, "expected identifier");
                Token name = previous();
                TokenKind op_kind = peek().kind;
                std::string op;
                if (op_kind == TokenKind::PlusEqual) op = "+";
                else if (op_kind == TokenKind::MinusEqual) op = "-";
                else if (op_kind == TokenKind::StarEqual) op = "*";
                else if (op_kind == TokenKind::SlashEqual) op = "/";
                else if (op_kind == TokenKind::PercentEqual) op = "%";
                else if (op_kind == TokenKind::AndEqual) op = "&";
                else if (op_kind == TokenKind::OrEqual) op = "|";
                else if (op_kind == TokenKind::CaretEqual) op = "^";
                else if (op_kind == TokenKind::LessLessEqual) op = "<<";
                else if (op_kind == TokenKind::GreaterGreaterEqual) op = ">>";
                else if (op_kind == TokenKind::StarStarEqual) op = "**";
                advance(); // consume the compound operator
                auto rhs = parse_expression();
                optional_semicolon();  // vNext: semicolons optional
                // Desugar to: name = name op rhs
                auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
                auto bin = std::make_unique<AstBinaryExpr>(op, name.line, name.column);
                bin->left = std::move(id);
                bin->right = std::move(rhs);
                auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
                a->target_name = name.lexeme;
                a->value = std::move(bin);
                return a;
            }
        }

        // block as statement
        if (check(TokenKind::LBrace)) {
            return parse_block();
        }

        // Check for member access or index assignment: expr.member = value or expr[index] = value
        if (check(TokenKind::Identifier)) {
            size_t saved = current_;
            auto lhs = parse_postfix();  // Parse left-hand side with member access

            if (match(TokenKind::Assign)) {
                auto rhs = parse_expression();
                optional_semicolon();  // vNext: semicolons optional
                auto a = std::make_unique<AstAssignStmt>(lhs->line, lhs->column);
                // Check if it's a simple identifier or complex expression
                if (auto* id = dynamic_cast<AstIdentifierExpr*>(lhs.get())) {
                    a->target_name = id->name;
                } else {
                    a->target_expr = std::move(lhs);
                }
                a->value = std::move(rhs);
                return a;
            }

            // Not an assignment, restore and fall through to expression statement
            current_ = saved;
        }

        return parse_expression_statement();
    }

    std::unique_ptr<AstStmt> Parser::parse_var_decl_statement() {
        expect(TokenKind::Identifier, "expected variable name");
        Token name = previous();
        expect(TokenKind::Colon, "expected ':'");
        std::string type = parse_type_name();
        expect(TokenKind::Assign, "expected '='");
        auto init = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto v = std::make_unique<AstVarDeclStmt>(name.line, name.column);
        v->name = name.lexeme;
        v->type_name = type;
        v->init_expr = std::move(init);
        return v;
    }

    std::unique_ptr<AstStmt> Parser::parse_destructure_statement(bool is_struct) {
        int l = peek().line;
        int c = peek().column;

        auto ds = std::make_unique<AstDestructureStmt>(l, c);
        ds->is_struct = is_struct;

        if (is_struct) {
            // {a, b, c}: Type = expr;
            expect(TokenKind::LBrace, "expected '{' for struct destructuring");
            while (!check(TokenKind::RBrace) && !is_at_end()) {
                expect(TokenKind::Identifier, "expected field name");
                Token field_name = previous();
                DestructureBinding binding;
                binding.field_name = field_name.lexeme;
                binding.name = field_name.lexeme;  // Same name by default
                binding.line = field_name.line;
                binding.column = field_name.column;
                ds->bindings.push_back(binding);

                if (!match(TokenKind::Comma)) break;
            }
            expect(TokenKind::RBrace, "expected '}' after destructuring pattern");
        } else {
            // [a, b, c]: [N]Type = expr;
            expect(TokenKind::LBracket, "expected '[' for array destructuring");
            int idx = 0;
            while (!check(TokenKind::RBracket) && !is_at_end()) {
                expect(TokenKind::Identifier, "expected variable name");
                Token var_name = previous();
                DestructureBinding binding;
                binding.name = var_name.lexeme;
                binding.field_name = std::to_string(idx++);  // Index as "field name"
                binding.line = var_name.line;
                binding.column = var_name.column;
                ds->bindings.push_back(binding);

                if (!match(TokenKind::Comma)) break;
            }
            expect(TokenKind::RBracket, "expected ']' after destructuring pattern");
        }

        expect(TokenKind::Colon, "expected ':' after destructuring pattern");
        ds->type_name = parse_type_name();
        expect(TokenKind::Assign, "expected '=' in destructuring statement");
        ds->init_expr = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        return ds;
    }

    std::unique_ptr<AstStmt> Parser::parse_let_statement() {
        // let name = expr;           (mutable, type inferred as "auto")
        // let name: type = expr;     (mutable, explicit type)
        // let mut name = expr;       (explicit mutable)
        // let mut name: type = expr; (explicit mutable with type)
        // let (a, b, c) = expr;      (tuple destructuring)

        // Optional 'mut' keyword (let is mutable by default, but allow explicit mut)
        match(TokenKind::KwMut);

        // Check for tuple destructuring pattern: let (a, b, c) = ...
        if (match(TokenKind::LParen)) {
            int l = previous().line;
            int c = previous().column;

            auto ds = std::make_unique<AstDestructureStmt>(l, c);
            ds->is_struct = false;
            ds->is_tuple = true;

            // Parse binding names
            int index = 0;
            if (!check(TokenKind::RParen)) {
                do {
                    expect(TokenKind::Identifier, "expected variable name in tuple pattern");
                    DestructureBinding binding;
                    binding.name = previous().lexeme;
                    binding.field_name = std::to_string(index++);  // Use index as "field name"
                    binding.line = previous().line;
                    binding.column = previous().column;
                    ds->bindings.push_back(binding);
                } while (match(TokenKind::Comma));
            }
            expect(TokenKind::RParen, "expected ')' after tuple pattern");

            // Optional type annotation
            if (match(TokenKind::Colon)) {
                ds->type_name = parse_type_name();
            } else {
                ds->type_name = "auto";
            }

            expect(TokenKind::Assign, "expected '='");
            ds->init_expr = parse_expression();
            optional_semicolon();  // vNext: semicolons optional

            return ds;
        }

        expect(TokenKind::Identifier, "expected variable name after 'let'");
        Token name = previous();

        std::string type = "auto"; // default to auto for type inference
        if (match(TokenKind::Colon)) {
            type = parse_type_name();
        }

        expect(TokenKind::Assign, "expected '='");
        auto init = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto v = std::make_unique<AstVarDeclStmt>(name.line, name.column);
        v->name = name.lexeme;
        v->type_name = type;
        v->init_expr = std::move(init);
        v->is_mutable = true;  // let is mutable by default
        return v;
    }

    std::unique_ptr<AstStmt> Parser::parse_const_statement() {
        // const name = expr;           (immutable, type inferred as "auto")
        // const name: type = expr;     (immutable, explicit type)

        expect(TokenKind::Identifier, "expected variable name after 'const'");
        Token name = previous();

        std::string type = "auto"; // default to auto for type inference
        if (match(TokenKind::Colon)) {
            type = parse_type_name();
        }

        expect(TokenKind::Assign, "expected '='");
        auto init = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto v = std::make_unique<AstVarDeclStmt>(name.line, name.column);
        v->name = name.lexeme;
        v->type_name = type;
        v->init_expr = std::move(init);
        v->is_mutable = false;  // const is immutable
        return v;
    }

    std::unique_ptr<AstStmt> Parser::parse_assign_statement() {
        expect(TokenKind::Identifier, "expected assignment target");
        Token name = previous();
        expect(TokenKind::Assign, "expected '='");
        auto rhs = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
        a->target_name = name.lexeme;
        a->value = std::move(rhs);
        return a;
    }

    std::unique_ptr<AstStmt> Parser::parse_scope_statement() {
        expect(TokenKind::Identifier, "expected scope name");
        Token name = previous();
        expect(TokenKind::Assign, "expected '='");
        auto init = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto s = std::make_unique<AstScopeStmt>(name.line, name.column);
        s->name = name.lexeme;
        s->init_expr = std::move(init);
        return s;
    }

    std::unique_ptr<AstStmt> Parser::parse_defer_statement() {
        auto b = parse_block();
        auto d = std::make_unique<AstDeferStmt>(b ? b->line : peek().line, b ? b->column : peek().column);
        d->body = std::move(b);
        return d;
    }

    std::unique_ptr<AstStmt> Parser::parse_return_statement() {
        int l = previous().line;
        int c = previous().column;

        auto r = std::make_unique<AstReturnStmt>(l, c);
        // Parse optional return value - check for tokens that end a statement
        if (!check(TokenKind::Semicolon) && !check(TokenKind::RBrace)) {
            r->value = parse_expression();
        }
        optional_semicolon();  // vNext: semicolons optional

        return r;
    }

    std::unique_ptr<AstStmt> Parser::parse_break_statement() {
        int l = previous().line;
        int c = previous().column;
        auto stmt = std::make_unique<AstBreakStmt>(l, c);
        // Parse optional break value - check for tokens that can start an expression
        // but not end a statement (like } or a statement keyword)
        if (!check(TokenKind::Semicolon) && !check(TokenKind::RBrace) &&
            !check(TokenKind::KwLet) && !check(TokenKind::KwReturn) &&
            !check(TokenKind::KwIf) && !check(TokenKind::KwWhile) &&
            !check(TokenKind::KwFor) && !check(TokenKind::KwBreak) &&
            !check(TokenKind::KwContinue)) {
            stmt->value = parse_expression();
        }
        optional_semicolon();  // vNext: semicolons optional
        return stmt;
    }

    std::unique_ptr<AstStmt> Parser::parse_continue_statement() {
        int l = previous().line;
        int c = previous().column;
        optional_semicolon();  // vNext: semicolons optional
        return std::make_unique<AstContinueStmt>(l, c);
    }

    std::unique_ptr<AstStmt> Parser::parse_if_statement() {
        int l = previous().line;
        int c = previous().column;

        auto i = std::make_unique<AstIfStmt>(l, c);

        // Check for if-let pattern: if let Some(x) = expr { ... }
        if (match(TokenKind::KwLet)) {
            i->is_if_let = true;

            // Parse pattern: Some(x), Ok(x), Err(e), None
            expect(TokenKind::Identifier, "expected pattern name (Some, Ok, Err, None)");
            i->pattern_kind = previous().lexeme;

            // Parse optional binding variable
            if (match(TokenKind::LParen)) {
                expect(TokenKind::Identifier, "expected variable name in pattern");
                i->pattern_var = previous().lexeme;
                expect(TokenKind::RParen, "expected ')' after pattern variable");
            }

            expect(TokenKind::Assign, "expected '=' after pattern");
            // Handle ambiguity: if expr is just an identifier followed by '{',
            // don't try to parse a struct literal - the '{' starts the block
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                i->pattern_expr = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                i->pattern_expr = parse_logical_or();  // Don't parse struct literals
            }
        } else {
            // Handle ambiguity: if condition is just an identifier followed by '{',
            // don't try to parse a struct literal - the '{' starts the block
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                i->condition = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                i->condition = parse_logical_or();  // Don't parse struct literals
            }
        }

        i->then_block = parse_block();

        if (match(TokenKind::KwElse)) {
            if (match(TokenKind::KwIf)) {
                i->else_block = parse_if_statement();
            } else {
                i->else_block = parse_block();
            }
        }

        return i;
    }

    std::unique_ptr<AstStmt> Parser::parse_while_statement() {
        int l = previous().line;
        int c = previous().column;

        auto w = std::make_unique<AstWhileStmt>(l, c);

        // Check for while-let pattern: while let Some(x) = expr { ... }
        if (match(TokenKind::KwLet)) {
            w->is_while_let = true;

            // Parse pattern: Some(x), Ok(x), Err(e)
            expect(TokenKind::Identifier, "expected pattern name (Some, Ok, Err)");
            w->pattern_kind = previous().lexeme;

            // Parse optional binding variable
            if (match(TokenKind::LParen)) {
                expect(TokenKind::Identifier, "expected variable name in pattern");
                w->pattern_var = previous().lexeme;
                expect(TokenKind::RParen, "expected ')' after pattern variable");
            }

            expect(TokenKind::Assign, "expected '=' after pattern");
            // Handle ambiguity with struct literals
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                w->pattern_expr = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                w->pattern_expr = parse_logical_or();
            }
        } else {
            // Handle ambiguity with struct literals for regular while
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                w->condition = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                w->condition = parse_logical_or();
            }
        }

        w->body = parse_block();
        return w;
    }

    std::unique_ptr<AstStmt> Parser::parse_loop_statement() {
        int l = previous().line;
        int c = previous().column;

        auto loop = std::make_unique<AstLoopStmt>(l, c);
        loop->body = parse_block();
        return loop;
    }

    std::unique_ptr<AstStmt> Parser::parse_for_statement() {
        int l = previous().line;
        int c = previous().column;

        // Check for destructuring for-in: for (key, value) in iterable { body }
        if (check(TokenKind::LParen)) {
            advance(); // consume '('
            std::vector<std::string> var_names;
            
            // Parse variable names
            while (!check(TokenKind::RParen) && !is_at_end()) {
                expect(TokenKind::Identifier, "expected variable name in destructuring");
                var_names.push_back(previous().lexeme);
                if (!match(TokenKind::Comma)) break;
            }
            expect(TokenKind::RParen, "expected ')' after destructuring pattern");
            expect(TokenKind::KwIn, "expected 'in' after destructuring pattern");

            // Parse iterable
            std::unique_ptr<AstNode> iterable;
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                iterable = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                iterable = parse_expression();
            }

            auto body = parse_block();

            auto fin = std::make_unique<AstForInStmt>(l, c);
            fin->var_names = std::move(var_names);
            fin->is_destructure = true;
            fin->iterable = std::move(iterable);
            fin->body = std::move(body);
            return fin;
        }

        // Check for for-in loop: for var in iterable { body }
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
            tokens_[current_ + 1].kind == TokenKind::KwIn) {
            // for-in loop
            expect(TokenKind::Identifier, "expected variable name");
            Token var_name = previous();
            expect(TokenKind::KwIn, "expected 'in'");

            // Parse iterable - but handle the case where identifier is followed by '{'
            // (which is the for-in body, not a struct literal)
            std::unique_ptr<AstNode> iterable;
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                // Simple identifier followed by '{' - parse as identifier, not struct literal
                advance();
                iterable = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                iterable = parse_expression();
            }

            auto body = parse_block();

            auto fin = std::make_unique<AstForInStmt>(l, c);
            fin->var_name = var_name.lexeme;
            fin->iterable = std::move(iterable);
            fin->body = std::move(body);
            return fin;
        }

        // Traditional for loop: for init; cond; step { body }
        // Example: for i: i32 = 0; i < 10; i = i + 1 { ... }

        // Parse init (var decl or assignment)
        std::unique_ptr<AstStmt> init;
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
            tokens_[current_ + 1].kind == TokenKind::Colon) {
            // var decl: i: i32 = 0
            expect(TokenKind::Identifier, "expected variable name");
            Token name = previous();
            expect(TokenKind::Colon, "expected ':'");
            std::string type = parse_type_name();
            expect(TokenKind::Assign, "expected '='");
            auto init_expr = parse_expression();

            auto v = std::make_unique<AstVarDeclStmt>(name.line, name.column);
            v->name = name.lexeme;
            v->type_name = type;
            v->init_expr = std::move(init_expr);
            init = std::move(v);
        } else if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                   tokens_[current_ + 1].kind == TokenKind::Assign) {
            // assignment: i = 0
            expect(TokenKind::Identifier, "expected assignment target");
            Token name = previous();
            expect(TokenKind::Assign, "expected '='");
            auto rhs = parse_expression();

            auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
            a->target_name = name.lexeme;
            a->value = std::move(rhs);
            init = std::move(a);
        }

        expect(TokenKind::Semicolon, "expected ';' after for init");

        // Parse condition
        auto cond = parse_expression();
        expect(TokenKind::Semicolon, "expected ';' after for condition");

        // Parse step (assignment, i++, i--, or compound assignment)
        std::unique_ptr<AstStmt> step;
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size())) {
            TokenKind next = tokens_[current_ + 1].kind;

            if (next == TokenKind::Assign) {
                // i = expr
                expect(TokenKind::Identifier, "expected assignment target");
                Token name = previous();
                expect(TokenKind::Assign, "expected '='");
                auto rhs = parse_expression();

                auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
                a->target_name = name.lexeme;
                a->value = std::move(rhs);
                step = std::move(a);
            }
            else if (next == TokenKind::PlusPlus) {
                // i++
                expect(TokenKind::Identifier, "expected identifier");
                Token name = previous();
                advance(); // consume ++
                // Desugar to: name = name + 1
                auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
                auto one = std::make_unique<AstLiteralExpr>("1", false, false, name.line, name.column);
                auto add = std::make_unique<AstBinaryExpr>("+", name.line, name.column);
                add->left = std::move(id);
                add->right = std::move(one);
                auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
                a->target_name = name.lexeme;
                a->value = std::move(add);
                step = std::move(a);
            }
            else if (next == TokenKind::MinusMinus) {
                // i--
                expect(TokenKind::Identifier, "expected identifier");
                Token name = previous();
                advance(); // consume --
                // Desugar to: name = name - 1
                auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
                auto one = std::make_unique<AstLiteralExpr>("1", false, false, name.line, name.column);
                auto sub = std::make_unique<AstBinaryExpr>("-", name.line, name.column);
                sub->left = std::move(id);
                sub->right = std::move(one);
                auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
                a->target_name = name.lexeme;
                a->value = std::move(sub);
                step = std::move(a);
            }
            else if (next == TokenKind::PlusEqual || next == TokenKind::MinusEqual ||
                     next == TokenKind::StarEqual || next == TokenKind::SlashEqual || next == TokenKind::PercentEqual ||
                     next == TokenKind::AndEqual || next == TokenKind::OrEqual || next == TokenKind::CaretEqual ||
                     next == TokenKind::LessLessEqual || next == TokenKind::GreaterGreaterEqual) {
                // i += expr, etc.
                expect(TokenKind::Identifier, "expected identifier");
                Token name = previous();
                TokenKind op_kind = peek().kind;
                std::string op;
                if (op_kind == TokenKind::PlusEqual) op = "+";
                else if (op_kind == TokenKind::MinusEqual) op = "-";
                else if (op_kind == TokenKind::StarEqual) op = "*";
                else if (op_kind == TokenKind::SlashEqual) op = "/";
                else if (op_kind == TokenKind::PercentEqual) op = "%";
                else if (op_kind == TokenKind::AndEqual) op = "&";
                else if (op_kind == TokenKind::OrEqual) op = "|";
                else if (op_kind == TokenKind::CaretEqual) op = "^";
                else if (op_kind == TokenKind::LessLessEqual) op = "<<";
                else if (op_kind == TokenKind::GreaterGreaterEqual) op = ">>";
                else if (op_kind == TokenKind::StarStarEqual) op = "**";
                advance(); // consume the compound operator
                auto rhs = parse_expression();
                // Desugar to: name = name op rhs
                auto id = std::make_unique<AstIdentifierExpr>(name.lexeme, name.line, name.column);
                auto bin = std::make_unique<AstBinaryExpr>(op, name.line, name.column);
                bin->left = std::move(id);
                bin->right = std::move(rhs);
                auto a = std::make_unique<AstAssignStmt>(name.line, name.column);
                a->target_name = name.lexeme;
                a->value = std::move(bin);
                step = std::move(a);
            }
        }

        // Parse body
        auto body = parse_block();

        auto f = std::make_unique<AstForStmt>(
            std::move(init),
            std::move(cond),
            std::move(step),
            std::move(body),
            l, c
        );
        return f;
    }

    std::unique_ptr<AstStmt> Parser::parse_expression_statement() {
        int l = peek().line;
        int c = peek().column;

        auto e = parse_expression();
        optional_semicolon();  // vNext: semicolons optional

        auto st = std::make_unique<AstExprStmt>(l, c);
        st->expr = std::move(e);
        return st;
    }

    // ---------------- expressions ----------------

    std::unique_ptr<AstExpr> Parser::parse_expression() {
        return parse_or_control_flow();
    }

    std::unique_ptr<AstExpr> Parser::parse_or_control_flow() {
        // Parse: expr or return/break/continue/{ block }
        // Very low precedence - binds loosest
        auto left = parse_null_coalesce();

        if (match(TokenKind::KwOr)) {
            int l = previous().line;
            int c = previous().column;
            auto or_expr = std::make_unique<AstOrExpr>(l, c);
            or_expr->lhs = std::move(left);

            // Check for block form: expr or { ... }
            if (check(TokenKind::LBrace)) {
                or_expr->fallback_block = parse_block();
            }
            // Check for return statement: expr or return [value]
            // Parse directly - don't use parse_return_statement() since it expects ';'
            else if (match(TokenKind::KwReturn)) {
                int rl = previous().line;
                int rc = previous().column;
                auto ret = std::make_unique<AstReturnStmt>(rl, rc);
                // Parse optional return value (don't expect semicolon)
                if (!check(TokenKind::Semicolon) && !check(TokenKind::RBrace)) {
                    ret->value = parse_expression();
                }
                or_expr->fallback_stmt = std::move(ret);
            }
            // Check for break statement: expr or break
            else if (match(TokenKind::KwBreak)) {
                or_expr->fallback_stmt = std::make_unique<AstBreakStmt>(previous().line, previous().column);
            }
            // Check for continue statement: expr or continue
            else if (match(TokenKind::KwContinue)) {
                or_expr->fallback_stmt = std::make_unique<AstContinueStmt>(previous().line, previous().column);
            }
            // Otherwise parse as default value expression: expr or default_value
            else {
                // Parse a simple expression as default value (use higher precedence to avoid infinite recursion)
                or_expr->default_expr = parse_null_coalesce();
            }

            return or_expr;
        }

        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_null_coalesce() {
        auto left = parse_logical_or();
        while (match(TokenKind::QuestionQuestion)) {
            Token op = previous();
            auto right = parse_logical_or();
            auto nc = std::make_unique<AstNullCoalesceExpr>(op.line, op.column);
            nc->option_expr = std::move(left);
            nc->default_expr = std::move(right);
            left = std::move(nc);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_logical_or() {
        auto left = parse_logical_and();
        while (match(TokenKind::OrOr)) {
            Token op = previous();
            auto right = parse_logical_and();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_logical_and() {
        auto left = parse_bitwise_or();
        while (match(TokenKind::AndAnd)) {
            Token op = previous();
            auto right = parse_bitwise_or();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_bitwise_or() {
        auto left = parse_bitwise_xor();
        while (match(TokenKind::Or)) {
            Token op = previous();
            auto right = parse_bitwise_xor();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_bitwise_xor() {
        auto left = parse_bitwise_and();
        while (match(TokenKind::Caret)) {
            Token op = previous();
            auto right = parse_bitwise_and();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_bitwise_and() {
        auto left = parse_equality();
        while (match(TokenKind::And)) {
            Token op = previous();
            auto right = parse_equality();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_equality() {
        auto left = parse_relational();
        while (match(TokenKind::EqualEqual) || match(TokenKind::BangEqual)) {
            Token op = previous();
            auto right = parse_relational();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_relational() {
        auto left = parse_shift();
        while (match(TokenKind::Less) || match(TokenKind::LessEqual) ||
            match(TokenKind::Greater) || match(TokenKind::GreaterEqual)) {
            Token op = previous();
            auto right = parse_shift();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_shift() {
        auto left = parse_additive();

        // Check for range operators (before shift operators)
        if (check(TokenKind::DotDot) || check(TokenKind::DotDotEqual)) {
            bool inclusive = check(TokenKind::DotDotEqual);
            Token range_tok = peek();
            advance();
            auto right = parse_additive();
            auto range = std::make_unique<AstRangeExpr>(range_tok.line, range_tok.column);
            range->start = std::move(left);
            range->end = std::move(right);
            range->inclusive = inclusive;
            return range;
        }

        while (match(TokenKind::LessLess) || match(TokenKind::GreaterGreater)) {
            Token op = previous();
            auto right = parse_additive();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_additive() {
        auto left = parse_multiplicative();
        while (match(TokenKind::Plus) || match(TokenKind::Minus)) {
            Token op = previous();
            auto right = parse_multiplicative();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_power() {
        // Power operator is right-associative
        auto left = parse_unary();
        if (match(TokenKind::StarStar)) {
            Token op = previous();
            auto right = parse_power();  // Right-associative: call self
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            return b;
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_multiplicative() {
        auto left = parse_power();
        while (match(TokenKind::Star) || match(TokenKind::Slash) || match(TokenKind::Percent)) {
            Token op = previous();
            auto right = parse_power();
            auto b = std::make_unique<AstBinaryExpr>(op.lexeme, op.line, op.column);
            b->left = std::move(left);
            b->right = std::move(right);
            left = std::move(b);
        }
        return left;
    }

    std::unique_ptr<AstExpr> Parser::parse_unary() {
        // Unary operators: !, -, ~, & (reference), &mut (mutable reference), * (dereference)
        if (match(TokenKind::And)) {
            Token op = previous();
            // Check for mutable reference: &mut expr
            if (match(TokenKind::KwMut)) {
                auto right = parse_unary();
                auto u = std::make_unique<AstUnaryExpr>("&mut", op.line, op.column);
                u->right = std::move(right);
                return u;
            }
            // Immutable reference: &expr
            auto right = parse_unary();
            auto u = std::make_unique<AstUnaryExpr>(op.lexeme, op.line, op.column);
            u->right = std::move(right);
            return u;
        }
        if (match(TokenKind::Bang) || match(TokenKind::Minus) || match(TokenKind::Tilde) ||
            match(TokenKind::Star)) {
            Token op = previous();
            auto right = parse_unary();
            auto u = std::make_unique<AstUnaryExpr>(op.lexeme, op.line, op.column);
            u->right = std::move(right);
            return u;
        }
        return parse_postfix();
    }

    std::unique_ptr<AstExpr> Parser::parse_postfix() {
        auto expr = parse_primary();

        while (true) {
            if (match(TokenKind::LBracket)) {
                int l = previous().line;
                int c = previous().column;
                auto index = parse_expression();
                expect(TokenKind::RBracket, "expected ']'");
                
                // Check if this is a slice expression (index is a range)
                if (index->kind == NodeKind::RangeExpr) {
                    auto range = dynamic_cast<AstRangeExpr*>(index.get());
                    auto slice_expr = std::make_unique<AstSliceExpr>(l, c);
                    slice_expr->base = std::move(expr);
                    slice_expr->start = std::move(range->start);
                    slice_expr->end = std::move(range->end);
                    slice_expr->inclusive = range->inclusive;
                    expr = std::move(slice_expr);
                } else {
                    auto idx_expr = std::make_unique<AstIndexExpr>(l, c);
                    idx_expr->base = std::move(expr);
                    idx_expr->index = std::move(index);
                    expr = std::move(idx_expr);
                }
            }
            else if (match(TokenKind::Dot)) {
                int l = previous().line;
                int c = previous().column;

                // Check for tuple index access: tuple.0, tuple.1, etc.
                if (match(TokenKind::IntLiteral)) {
                    Token idx_token = previous();
                    int index = std::stoi(idx_token.lexeme);
                    auto tuple_idx = std::make_unique<AstTupleIndexExpr>(index, l, c);
                    tuple_idx->tuple = std::move(expr);
                    expr = std::move(tuple_idx);
                    continue;
                }

                // Check for await: expr.await
                if (match(TokenKind::KwAwait)) {
                    auto await_expr = std::make_unique<AstAwaitExpr>(l, c);
                    await_expr->operand = std::move(expr);
                    expr = std::move(await_expr);
                    continue;
                }

                expect(TokenKind::Identifier, "expected member name after '.'");
                Token member = previous();

                // Check if it's a method call: expr.method(...)
                if (match(TokenKind::LParen)) {
                    auto method_call = std::make_unique<AstMethodCallExpr>(member.lexeme, l, c);
                    method_call->object = std::move(expr);

                    if (!check(TokenKind::RParen)) {
                        while (true) {
                            // Check for named argument: name: value
                            if (check(TokenKind::Identifier) && check_next(TokenKind::Colon)) {
                                Token arg_name = advance();  // consume identifier
                                advance();  // consume colon
                                method_call->arg_names.push_back(arg_name.lexeme);
                                method_call->args.push_back(parse_expression());
                            } else {
                                method_call->arg_names.push_back("");  // positional argument
                                method_call->args.push_back(parse_expression());
                            }
                            if (match(TokenKind::Comma)) continue;
                            break;
                        }
                    }

                    expect(TokenKind::RParen, "expected ')'");
                    expr = std::move(method_call);
                } else {
                    // Just member access
                    auto member_expr = std::make_unique<AstMemberAccessExpr>(member.lexeme, l, c);
                    member_expr->object = std::move(expr);
                    expr = std::move(member_expr);
                }
            }
            else if (match(TokenKind::LParen)) {
                // function call on expression result
                int l = previous().line;
                int c = previous().column;
                // For now, only support calls on identifiers
                // This handles chaining like arr[0]() but we'll need to track the callee
                // For simplicity, we'll handle direct identifier calls in parse_primary
                // and reject other call patterns here
                diag_.error("function call on non-identifier not supported", l, c);
                return expr;
            }
            else if (match(TokenKind::QuestionDot)) {
                // Optional chaining: expr?.field or expr?.method()
                int l = previous().line;
                int c = previous().column;
                
                expect(TokenKind::Identifier, "expected identifier after '?.'");
                Token member = previous();
                
                auto chain_expr = std::make_unique<AstOptionalChainExpr>(member.lexeme, l, c);
                chain_expr->object = std::move(expr);
                
                // Check if it's a method call: expr?.method(...)
                if (match(TokenKind::LParen)) {
                    chain_expr->is_method_call = true;
                    
                    if (!check(TokenKind::RParen)) {
                        while (true) {
                            // Check for named argument: name: value
                            if (check(TokenKind::Identifier) && check_next(TokenKind::Colon)) {
                                Token arg_name = advance();  // consume identifier
                                advance();  // consume colon
                                chain_expr->arg_names.push_back(arg_name.lexeme);
                                chain_expr->args.push_back(parse_expression());
                            } else {
                                chain_expr->arg_names.push_back("");  // positional argument
                                chain_expr->args.push_back(parse_expression());
                            }
                            if (match(TokenKind::Comma)) continue;
                            break;
                        }
                    }
                    expect(TokenKind::RParen, "expected ')'");
                }
                
                expr = std::move(chain_expr);
            }
            else if (match(TokenKind::Question)) {
                // Try expression: expr? for error propagation
                int l = previous().line;
                int c = previous().column;
                auto try_expr = std::make_unique<AstTryExpr>(l, c);
                try_expr->operand = std::move(expr);
                expr = std::move(try_expr);
            }
            else if (match(TokenKind::KwAs)) {
                // Cast expression: expr as Type
                int l = previous().line;
                int c = previous().column;
                std::string type_name = parse_type_name();
                auto cast_expr = std::make_unique<AstCastExpr>(l, c);
                cast_expr->operand = std::move(expr);
                cast_expr->target_type = type_name;
                expr = std::move(cast_expr);
            }
            else {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<AstExpr> Parser::parse_primary() {
        if (match(TokenKind::IntLiteral) || match(TokenKind::FloatLiteral)) {
            Token lit = previous();
            return std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column);
        }

        if (match(TokenKind::StringLiteral) || match(TokenKind::RawStringLiteral) || match(TokenKind::MultiLineStringLiteral)) {
            Token lit = previous();
            return std::make_unique<AstLiteralExpr>(lit.lexeme, true, false, lit.line, lit.column);
        }

        if (match(TokenKind::CharLiteral)) {
            Token lit = previous();
            return std::make_unique<AstLiteralExpr>(lit.lexeme, false, true, lit.line, lit.column);
        }

        // If expression: if cond { expr } else { expr }
        if (match(TokenKind::KwIf)) {
            Token if_tok = previous();
            // Parse condition - use parse_logical_or to avoid ambiguity with struct literals
            std::unique_ptr<AstExpr> cond;
            if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
                tokens_[current_ + 1].kind == TokenKind::LBrace) {
                advance();
                cond = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
            } else {
                cond = parse_logical_or();
            }
            expect(TokenKind::LBrace, "expected '{' after if condition");
            auto then_expr = parse_expression();
            expect(TokenKind::RBrace, "expected '}' after then expression");
            expect(TokenKind::KwElse, "if expression requires else branch");
            expect(TokenKind::LBrace, "expected '{' after else");
            auto else_expr = parse_expression();
            expect(TokenKind::RBrace, "expected '}' after else expression");
            auto ie = std::make_unique<AstIfExpr>(if_tok.line, if_tok.column);
            ie->condition = std::move(cond);
            ie->then_expr = std::move(then_expr);
            ie->else_expr = std::move(else_expr);
            return ie;
        }

        if (match(TokenKind::KwTrue) || match(TokenKind::KwFalse)) {
            Token t = previous();
            return std::make_unique<AstLiteralExpr>(t.lexeme, false, false, t.line, t.column);
        }

        if (match(TokenKind::KwSelf)) {
            Token t = previous();
            return std::make_unique<AstSelfExpr>(t.line, t.column);
        }

        if (match(TokenKind::KwNone)) {
            Token t = previous();
            return std::make_unique<AstNoneExpr>(t.line, t.column);
        }

        if (match(TokenKind::KwMatch)) {
            return parse_match_expression(false);  // match style: => and commas
        }

        if (match(TokenKind::KwWhen)) {
            return parse_match_expression(true);   // when style: -> and newlines
        }

        if (match(TokenKind::LParen)) {
            int l = previous().line;
            int c = previous().column;

            // Empty tuple: ()
            if (match(TokenKind::RParen)) {
                auto tuple = std::make_unique<AstTupleExpr>(l, c);
                return tuple;
            }

            auto first_expr = parse_expression();

            // Check for tuple: (expr, ...) or (expr,)
            if (match(TokenKind::Comma)) {
                auto tuple = std::make_unique<AstTupleExpr>(l, c);
                tuple->elements.push_back(std::move(first_expr));

                // Handle trailing comma or more elements
                if (!check(TokenKind::RParen)) {
                    tuple->elements.push_back(parse_expression());
                    while (match(TokenKind::Comma)) {
                        if (check(TokenKind::RParen)) break;  // trailing comma
                        tuple->elements.push_back(parse_expression());
                    }
                }
                expect(TokenKind::RParen, "expected ')' after tuple elements");
                return tuple;
            }

            // Just grouping: (expr)
            expect(TokenKind::RParen, "expected ')'");
            return first_expr;
        }

        // move closure: move |params| expr
        if (match(TokenKind::KwMove)) {
            if (check(TokenKind::Or)) {
                auto closure = parse_closure_expression();
                auto cl = static_cast<AstClosureExpr*>(closure.get());
                cl->captures_by_ref = false;  // move captures by value
                return closure;
            }
            // Not a closure, this is an error or different construct
            diag_.error("expected '|' after 'move' for closure", previous().line, previous().column);
        }

        // Explicit capture list: [x, &y, move z]|params| expr
        if (check(TokenKind::LBracket)) {
            // Look ahead to see if this is a capture list followed by |
            // We need to distinguish [capture_list]| from [array_literal]
            size_t saved = current_;
            advance(); // consume [
            
            // Check if this looks like a capture list: identifier, &identifier, or move identifier
            bool is_capture_list = false;
            if (check(TokenKind::And) || check(TokenKind::KwMove) || 
                (check(TokenKind::Identifier) && !check_next(TokenKind::Semicolon))) {
                // Scan ahead to find ] followed by |
                int bracket_depth = 1;
                size_t scan = current_;
                while (scan < tokens_.size() && bracket_depth > 0) {
                    if (tokens_[scan].kind == TokenKind::LBracket) bracket_depth++;
                    else if (tokens_[scan].kind == TokenKind::RBracket) bracket_depth--;
                    scan++;
                }
                // Check if ] is followed by |
                if (scan < tokens_.size() && tokens_[scan].kind == TokenKind::Or) {
                    is_capture_list = true;
                }
            }
            
            current_ = saved;  // Restore position
            
            if (is_capture_list) {
                return parse_closure_with_captures();
            }
        }

        // Closure: |params| expr or |params| { block }
        if (match(TokenKind::Or)) {
            // Check for empty params: ||
            if (check(TokenKind::Or)) {
                advance(); // consume the second |
                auto closure = std::make_unique<AstClosureExpr>(previous().line, previous().column);
                // Parse body
                if (check(TokenKind::LBrace)) {
                    closure->body_block = parse_block();
                } else {
                    closure->body_expr = parse_expression();
                }
                return closure;
            }
            // Put it back and parse as closure with params
            current_--; // unconsume the |
        }

        if (check(TokenKind::Or)) {
            return parse_closure_expression();
        }

        // Array literal: [expr, expr, ...] or [value; count]
        if (match(TokenKind::LBracket)) {
            int l = previous().line;
            int c = previous().column;
            auto arr = std::make_unique<AstArrayLiteralExpr>(l, c);

            if (!check(TokenKind::RBracket)) {
                auto first = parse_expression();
                
                // Check for fill syntax: [value; count]
                if (match(TokenKind::Semicolon)) {
                    arr->fill_value = std::move(first);
                    arr->fill_count = parse_expression();
                } else {
                    // Regular array literal
                    arr->elements.push_back(std::move(first));
                    while (match(TokenKind::Comma)) {
                        if (check(TokenKind::RBracket)) break;  // trailing comma
                        arr->elements.push_back(parse_expression());
                    }
                }
            }

            expect(TokenKind::RBracket, "expected ']'");
            return arr;
        }

        if (match(TokenKind::Identifier)) {
            Token id = previous();

            // Scope access: Enum::Variant or Module::Type or static method call Type::func()
            if (match(TokenKind::ColonColon)) {
                expect(TokenKind::Identifier, "expected identifier after '::'");
                Token member = previous();
                
                // Check for static method call: Type::func()
                if (match(TokenKind::LParen)) {
                    // Static method call like HashMap::new()
                    std::string qualified_name = id.lexeme + "::" + member.lexeme;
                    auto call = std::make_unique<AstCallExpr>(qualified_name, id.line, id.column);
                    
                    if (!check(TokenKind::RParen)) {
                        while (true) {
                            // Check for named argument: name: value
                            if (check(TokenKind::Identifier) && check_next(TokenKind::Colon)) {
                                Token arg_name = advance();  // consume identifier
                                advance();  // consume colon
                                call->arg_names.push_back(arg_name.lexeme);
                                call->args.push_back(parse_expression());
                            } else {
                                call->arg_names.push_back("");  // positional argument
                                call->args.push_back(parse_expression());
                            }
                            if (match(TokenKind::Comma)) continue;
                            break;
                        }
                    }
                    
                    expect(TokenKind::RParen, "expected ')'");
                    return call;
                }
                
                return std::make_unique<AstScopeAccessExpr>(id.lexeme, member.lexeme, id.line, id.column);
            }

            // call?
            if (match(TokenKind::LParen)) {
                auto call = std::make_unique<AstCallExpr>(id.lexeme, id.line, id.column);

                if (!check(TokenKind::RParen)) {
                    while (true) {
                        // Check for named argument: name: value
                        if (check(TokenKind::Identifier) && check_next(TokenKind::Colon)) {
                            Token arg_name = advance();  // consume identifier
                            advance();  // consume colon
                            call->arg_names.push_back(arg_name.lexeme);
                            call->args.push_back(parse_expression());
                        } else {
                            call->arg_names.push_back("");  // positional argument
                            call->args.push_back(parse_expression());
                        }
                        if (match(TokenKind::Comma)) continue;
                        break;
                    }
                }

                expect(TokenKind::RParen, "expected ')'");
                return call;
            }

            // struct literal: TypeName{} or TypeName{field: value, ...} or TypeName{value, ...}
            // Also handle generic struct literals: TypeName<T>{...}
            // Only treat as struct literal if the identifier looks like a type name
            // (starts with uppercase letter or is a known type like Vec, Option, Result)
            bool looks_like_type = !id.lexeme.empty() && 
                (std::isupper(id.lexeme[0]) || id.lexeme == "Vec" || 
                 id.lexeme == "Option" || id.lexeme == "Result" || id.lexeme == "HashMap");
            if (looks_like_type && (check(TokenKind::LBrace) || check(TokenKind::Less))) {
                std::string type_name = id.lexeme;

                // Parse optional type arguments for generic struct literals
                if (match(TokenKind::Less)) {
                    type_name += "<";
                    type_name += parse_type_name();
                    while (match(TokenKind::Comma)) {
                        type_name += ", ";
                        type_name += parse_type_name();
                    }
                    expect(TokenKind::Greater, "expected '>' after type arguments");
                    type_name += ">";

                    // After type args, must have '{' for struct literal
                    if (!check(TokenKind::LBrace)) {
                        diag_.error("expected '{' after generic type", peek().line, peek().column);
                        return std::make_unique<AstIdentifierExpr>(id.lexeme, id.line, id.column);
                    }
                }

                if (match(TokenKind::LBrace)) {
                    auto lit = std::make_unique<AstStructLiteralExpr>(type_name, id.line, id.column);

                    if (!check(TokenKind::RBrace)) {
                        // Check if first field is named (ident:) or positional
                        bool first = true;
                        while (true) {
                            if (first && check(TokenKind::Identifier) &&
                                (current_ + 1 < tokens_.size()) && tokens_[current_ + 1].kind == TokenKind::Colon) {
                                lit->is_named = true;
                            }
                            first = false;

                            AstStructFieldInit field_init;
                            field_init.line = peek().line;
                            field_init.column = peek().column;

                            if (lit->is_named) {
                                expect(TokenKind::Identifier, "expected field name");
                                field_init.field_name = previous().lexeme;
                                expect(TokenKind::Colon, "expected ':' after field name");
                            }

                            field_init.value = parse_expression();
                            lit->fields.push_back(std::move(field_init));

                            if (match(TokenKind::Comma)) continue;
                            break;
                        }
                    }

                    expect(TokenKind::RBrace, "expected '}'");
                    return lit;
                }
            }

            return std::make_unique<AstIdentifierExpr>(id.lexeme, id.line, id.column);
        }

        diag_.error("expected expression", peek().line, peek().column);
        return std::make_unique<AstLiteralExpr>("0", false, false, peek().line, peek().column); // recover
    }

    std::unique_ptr<AstExpr> Parser::parse_match_expression(bool is_when_style) {
        // match (value) { pattern => expr, ... }        (match style)
        // when value { pattern -> expr ... }            (when style, vNext)
        // OR match/when value { ... } where value is a simple expression (not struct literal)
        int l = previous().line;
        int c = previous().column;

        auto match_expr = std::make_unique<AstMatchExpr>(l, c);
        match_expr->declared_as_when = is_when_style;

        // Parse the match value - handle ambiguity with struct literals
        // If we see an identifier followed by '{', it's the match body, not a struct literal
        if (check(TokenKind::Identifier) && (current_ + 1 < tokens_.size()) &&
            tokens_[current_ + 1].kind == TokenKind::LBrace) {
            // Simple identifier - just parse it directly
            advance();
            match_expr->value = std::make_unique<AstIdentifierExpr>(previous().lexeme, previous().line, previous().column);
        } else if (match(TokenKind::LParen)) {
            // Parenthesized expression
            match_expr->value = parse_expression();
            expect(TokenKind::RParen, "expected ')' after match value");
        } else {
            // Other expressions (member access, etc.) - parse normally but stop at '{'
            match_expr->value = parse_logical_or();  // Don't parse struct literals
        }

        expect(TokenKind::LBrace, "expected '{' after match value");

        while (!check(TokenKind::RBrace) && !is_at_end()) {
            AstMatchArm arm;
            arm.line = peek().line;
            arm.column = peek().column;

            // Parse pattern: literal, identifier (including _), enum variant, or Option/Result pattern
            if (match(TokenKind::KwNone)) {
                // None pattern for Option matching
                Token pat = previous();
                arm.patterns.push_back(std::make_unique<AstOptionPattern>("None", "", pat.line, pat.column));
            } else if (match(TokenKind::Underscore)) {
                // Underscore wildcard pattern
                Token pat = previous();
                match_expr->has_default = true;
                arm.patterns.push_back(std::make_unique<AstIdentifierExpr>("_", pat.line, pat.column));
            } else if (match(TokenKind::Identifier)) {
                Token pat = previous();
                if (pat.lexeme == "_") {
                    // Wildcard pattern - represents default case
                    match_expr->has_default = true;
                    arm.patterns.push_back(std::make_unique<AstIdentifierExpr>("_", pat.line, pat.column));
                } else if ((pat.lexeme == "Some" || pat.lexeme == "Ok" || pat.lexeme == "Err" ||
                           pat.lexeme == "some" || pat.lexeme == "ok" || pat.lexeme == "err") && check(TokenKind::LParen)) {
                    // Option/Result pattern: Some(x), Ok(x), Err(e) or lowercase variants ok(x), err(e)
                    advance();  // consume '('
                    expect(TokenKind::Identifier, "expected binding variable name in pattern");
                    std::string binding = previous().lexeme;
                    expect(TokenKind::RParen, "expected ')' after binding variable");
                    arm.patterns.push_back(std::make_unique<AstOptionPattern>(pat.lexeme, binding, pat.line, pat.column));
                } else if (match(TokenKind::ColonColon)) {
                    // Enum variant pattern: Enum::Variant or Enum::Variant(x, y) or Enum::Variant { f: x }
                    expect(TokenKind::Identifier, "expected variant name after '::'");
                    Token variant = previous();
                    auto enum_pat = std::make_unique<AstEnumPattern>(pat.lexeme, variant.lexeme, pat.line, pat.column);
                    
                    if (match(TokenKind::LParen)) {
                        // Tuple destructuring: Enum::Variant(x, y)
                        enum_pat->is_tuple_pattern = true;
                        while (!check(TokenKind::RParen) && !is_at_end()) {
                            if (match(TokenKind::Underscore)) {
                                enum_pat->bindings.push_back("_");
                            } else {
                                expect(TokenKind::Identifier, "expected binding name in pattern");
                                enum_pat->bindings.push_back(previous().lexeme);
                            }
                            if (!check(TokenKind::RParen)) {
                                expect(TokenKind::Comma, "expected ',' between pattern bindings");
                            }
                        }
                        expect(TokenKind::RParen, "expected ')' after pattern bindings");
                    } else if (match(TokenKind::LBrace)) {
                        // Struct destructuring: Enum::Variant { field: x }
                        enum_pat->is_tuple_pattern = false;
                        while (!check(TokenKind::RBrace) && !is_at_end()) {
                            expect(TokenKind::Identifier, "expected field name in pattern");
                            std::string field_name = previous().lexeme;
                            std::string binding_name = field_name;  // Default: field name is binding
                            if (match(TokenKind::Colon)) {
                                expect(TokenKind::Identifier, "expected binding name after ':'");
                                binding_name = previous().lexeme;
                            }
                            enum_pat->field_bindings.push_back({field_name, binding_name});
                            if (!check(TokenKind::RBrace)) {
                                expect(TokenKind::Comma, "expected ',' between field bindings");
                            }
                        }
                        expect(TokenKind::RBrace, "expected '}' after field bindings");
                    }
                    // Unit pattern (no bindings) - just Enum::Variant
                    arm.patterns.push_back(std::move(enum_pat));
                } else {
                    // Check if this is a binding pattern (identifier followed by 'if' guard or '=>')
                    if (check(TokenKind::KwIf) || check(TokenKind::FatArrow)) {
                        // This is a binding pattern - captures the value into 'pat.lexeme'
                        arm.binding = pat.lexeme;
                        match_expr->has_default = true;
                    } else {
                        // Regular identifier pattern (for constant/enum comparison)
                        arm.patterns.push_back(std::make_unique<AstIdentifierExpr>(pat.lexeme, pat.line, pat.column));
                    }
                }
            } else if (match(TokenKind::IntLiteral) || match(TokenKind::FloatLiteral)) {
                Token lit = previous();
                // Check for range pattern: 0..5 or 0..=5
                if (match(TokenKind::DotDot) || match(TokenKind::DotDotEqual)) {
                    bool inclusive = (previous().kind == TokenKind::DotDotEqual);
                    auto range_pat = std::make_unique<AstRangeExpr>(lit.line, lit.column);
                    range_pat->start = std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column);
                    range_pat->inclusive = inclusive;
                    if (match(TokenKind::IntLiteral) || match(TokenKind::FloatLiteral)) {
                        Token end_lit = previous();
                        range_pat->end = std::make_unique<AstLiteralExpr>(end_lit.lexeme, false, false, end_lit.line, end_lit.column);
                    } else {
                        diag_.error("expected number after range operator", peek().line, peek().column);
                    }
                    arm.patterns.push_back(std::move(range_pat));
                } else {
                    arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column));
                }
            } else if (match(TokenKind::StringLiteral)) {
                Token lit = previous();
                arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, true, false, lit.line, lit.column));
            } else if (match(TokenKind::KwTrue) || match(TokenKind::KwFalse)) {
                Token lit = previous();
                arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column));
            } else {
                diag_.error("expected pattern in match arm", peek().line, peek().column);
                return match_expr;
            }

            // Check for or-patterns: pattern1 | pattern2 | ...
            while (match(TokenKind::Or)) {
                // Parse another pattern
                if (match(TokenKind::KwNone)) {
                    Token pat = previous();
                    arm.patterns.push_back(std::make_unique<AstOptionPattern>("None", "", pat.line, pat.column));
                } else if (match(TokenKind::IntLiteral) || match(TokenKind::FloatLiteral)) {
                    Token lit = previous();
                    arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column));
                } else if (match(TokenKind::StringLiteral)) {
                    Token lit = previous();
                    arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, true, false, lit.line, lit.column));
                } else if (match(TokenKind::KwTrue) || match(TokenKind::KwFalse)) {
                    Token lit = previous();
                    arm.patterns.push_back(std::make_unique<AstLiteralExpr>(lit.lexeme, false, false, lit.line, lit.column));
                } else if (match(TokenKind::Identifier)) {
                    Token pat = previous();
                    if (pat.lexeme == "_") {
                        match_expr->has_default = true;
                        arm.patterns.push_back(std::make_unique<AstIdentifierExpr>("_", pat.line, pat.column));
                    } else {
                        arm.patterns.push_back(std::make_unique<AstIdentifierExpr>(pat.lexeme, pat.line, pat.column));
                    }
                } else {
                    diag_.error("expected pattern after '|'", peek().line, peek().column);
                    break;
                }
            }

            // Parse optional guard clause: if condition
            if (match(TokenKind::KwIf)) {
                arm.guard = parse_expression();
            }

            // when uses ->, match uses =>
            if (is_when_style) {
                expect(TokenKind::Arrow, "expected '->' after pattern");
            } else {
                expect(TokenKind::FatArrow, "expected '=>' after pattern");
            }

            // Parse result expression
            arm.result = parse_expression();
            match_expr->arms.push_back(std::move(arm));

            // when style: newlines separate arms (commas optional)
            // match style: commas required between arms
            if (!check(TokenKind::RBrace)) {
                if (is_when_style) {
                    match(TokenKind::Comma);  // Optional comma in when style
                } else {
                    expect(TokenKind::Comma, "expected ',' after match arm");
                }
            } else {
                match(TokenKind::Comma); // allow trailing comma
            }
        }

        expect(TokenKind::RBrace, "expected '}' after match arms");
        return match_expr;
    }

    std::unique_ptr<AstExpr> Parser::parse_closure_expression() {
        // Closure: |param1, param2: type| expr or |params| { block }
        // Supports underscore for unused params: |_| or |_, x|
        expect(TokenKind::Or, "expected '|' to start closure");
        int l = previous().line;
        int c = previous().column;

        auto closure = std::make_unique<AstClosureExpr>(l, c);

        // Parse parameters
        if (!check(TokenKind::Or)) {
            int underscore_count = 0;
            while (true) {
                AstClosureParam param;
                param.line = peek().line;
                param.column = peek().column;

                // Accept underscore or identifier for param name
                if (match(TokenKind::Underscore)) {
                    // Generate unique name for underscore params
                    param.name = "_unused_" + std::to_string(underscore_count++);
                } else {
                    expect(TokenKind::Identifier, "expected parameter name or '_'");
                    param.name = previous().lexeme;
                }

                // Optional type annotation
                if (match(TokenKind::Colon)) {
                    param.type_name = parse_type_name();
                }

                closure->params.push_back(param);

                if (match(TokenKind::Comma)) continue;
                break;
            }
        }

        expect(TokenKind::Or, "expected '|' after closure parameters");

        // Optional return type: |x| -> i32 { ... }
        if (match(TokenKind::Arrow)) {
            closure->return_type = parse_type_name();
        }

        // Parse body
        if (check(TokenKind::LBrace)) {
            closure->body_block = parse_block();
        } else {
            closure->body_expr = parse_expression();
        }

        return closure;
    }

    std::unique_ptr<AstExpr> Parser::parse_closure_with_captures() {
        // Explicit capture list: [x, &y, move z]|params| expr
        expect(TokenKind::LBracket, "expected '[' for capture list");
        int l = previous().line;
        int c = previous().column;

        auto closure = std::make_unique<AstClosureExpr>(l, c);
        closure->has_explicit_captures = true;

        // Parse capture list
        if (!check(TokenKind::RBracket)) {
            while (true) {
                CaptureSpec cap;
                
                if (match(TokenKind::And)) {
                    // &x - capture by reference
                    cap.mode = CaptureMode::ByRef;
                    expect(TokenKind::Identifier, "expected identifier after '&' in capture");
                    cap.name = previous().lexeme;
                } else if (match(TokenKind::KwMove)) {
                    // move x - capture by move
                    cap.mode = CaptureMode::ByMove;
                    expect(TokenKind::Identifier, "expected identifier after 'move' in capture");
                    cap.name = previous().lexeme;
                } else if (match(TokenKind::Identifier)) {
                    // x - capture by value (copy)
                    cap.mode = CaptureMode::ByValue;
                    cap.name = previous().lexeme;
                } else {
                    diag_.error("expected capture specification", peek().line, peek().column);
                    break;
                }

                closure->captures.push_back(cap);

                if (match(TokenKind::Comma)) continue;
                break;
            }
        }

        expect(TokenKind::RBracket, "expected ']' after capture list");

        // Now parse the closure parameters
        expect(TokenKind::Or, "expected '|' after capture list");

        // Parse parameters (same logic as parse_closure_expression)
        if (!check(TokenKind::Or)) {
            int underscore_count = 0;
            while (true) {
                AstClosureParam param;
                param.line = peek().line;
                param.column = peek().column;

                if (match(TokenKind::Underscore)) {
                    param.name = "_unused_" + std::to_string(underscore_count++);
                } else {
                    expect(TokenKind::Identifier, "expected parameter name or '_'");
                    param.name = previous().lexeme;
                }

                if (match(TokenKind::Colon)) {
                    param.type_name = parse_type_name();
                }

                closure->params.push_back(param);

                if (match(TokenKind::Comma)) continue;
                break;
            }
        }

        expect(TokenKind::Or, "expected '|' after closure parameters");

        // Optional return type
        if (match(TokenKind::Arrow)) {
            closure->return_type = parse_type_name();
        }

        // Parse body
        if (check(TokenKind::LBrace)) {
            closure->body_block = parse_block();
        } else {
            closure->body_expr = parse_expression();
        }

        return closure;
    }

} // namespace mana::frontend
