#pragma once

#include <string>
#include <memory>
#include <vector>

#include "AstNodes.h"

// forward declarations to avoid include cycles
namespace mana::frontend {
    struct AstBlockStmt;
    struct AstVarDeclStmt;
    struct AstExpr;
}

namespace mana::frontend {

    // Type constraint for where clauses: T: Trait1 + Trait2
    struct TypeConstraint {
        std::string type_param;           // The type parameter (e.g., "T")
        std::vector<std::string> traits;  // Required traits (e.g., ["Describable", "Clone"])
        int line = 0;
        int column = 0;
    };

    struct AstParam {
        std::string name;
        std::string type_name;
        std::unique_ptr<AstExpr> default_value;  // Optional default value
        int line = 0;
        int column = 0;
        
        bool has_default() const { return default_value != nullptr; }
    };

    struct AstDecl : AstNode {
        std::string source_module;  // Module this declaration came from (for imports)
        std::string doc_comment;    // Documentation comment (/// comment text)

        explicit AstDecl(NodeKind kind, int line = 0, int column = 0)
            : AstNode(kind, line, column) {
        }
        virtual ~AstDecl() = default;

        // Override in subclasses that support visibility
        virtual bool is_public() const { return false; }
        bool has_doc() const { return !doc_comment.empty(); }
    };

    struct AstImportDecl : AstDecl {
        std::string name;
        std::string path;           // File path for file imports
        bool is_file_import = false; // true for "import foo" (file), false for "import std::io"

        explicit AstImportDecl(std::string n, int line = 0, int column = 0)
            : AstDecl(NodeKind::ImportDecl, line, column),
            name(std::move(n)) {
        }
    };

    // use std::io;
    // use std::io::{read, write};
    // use std::io::* as io;
    // pub use std::io;
    struct AstUseDecl : AstDecl {
        std::string module_path;                    // e.g., "std::io"
        std::vector<std::string> imported_names;    // e.g., ["read", "write"] for selective import
        std::string alias;                          // e.g., "io" for "use std::io as io"
        bool is_glob = false;                       // true for "use std::io::*"
        bool is_pub = false;                        // true for "pub use"

        explicit AstUseDecl(std::string path, int line = 0, int column = 0)
            : AstDecl(NodeKind::UseDecl, line, column),
            module_path(std::move(path)) {
        }
    };

    struct AstFuncDecl : AstDecl {
        std::string name;
        std::string receiver_type;  // Non-empty for methods (e.g., "Vec3" for fn Vec3.length())
        std::vector<std::string> type_params;  // Generic type parameters (e.g., ["T", "U"])
        std::vector<TypeConstraint> constraints;  // Where clause constraints
        std::vector<AstParam> params;
        std::string return_type;
        std::unique_ptr<AstBlockStmt> body;
        bool is_pub = false;  // pub fn
        bool is_async = false;  // async fn
        bool is_static = false;  // static fn (no self parameter)
        bool is_test = false;  // #[test] fn
        bool is_extern = false;  // extern fn (FFI declaration)
        bool has_self = false;  // method has 'self' parameter

        bool is_method() const { return !receiver_type.empty(); }
        bool is_instance_method() const { return has_self; }
        bool is_generic() const { return !type_params.empty(); }
        bool has_constraints() const { return !constraints.empty(); }
        bool is_public() const override { return is_pub; }

        explicit AstFuncDecl(std::string n, int line = 0, int column = 0)
            : AstDecl(NodeKind::FunctionDecl, line, column),
            name(std::move(n)) {
        }
    };

    struct AstGlobalVarDecl : AstDecl {
        // holds the inner var-decl statement (name/type/init)
        std::unique_ptr<AstVarDeclStmt> var;

        explicit AstGlobalVarDecl(int line = 0, int column = 0)
            : AstDecl(NodeKind::GlobalVarDecl, line, column) {
        }
    };

    struct AstStructField {
        std::string name;
        std::string type_name;
        std::unique_ptr<AstExpr> default_value;  // Optional default value
        int line = 0;
        int column = 0;
    };

    struct AstStructDecl : AstDecl {
        std::string name;
        std::vector<std::string> type_params;  // Generic type parameters
        std::vector<AstStructField> fields;
        bool is_pub = false;  // pub struct

        bool is_generic() const { return !type_params.empty(); }
        bool is_public() const override { return is_pub; }

        explicit AstStructDecl(std::string n, int line = 0, int column = 0)
            : AstDecl(NodeKind::StructDecl, line, column),
            name(std::move(n)) {
        }
    };

    struct AstEnumVariant {
        std::string name;
        bool has_value = false;      // Has explicit integer value (C-style)
        int value = 0;

        // Associated data for algebraic data types
        std::vector<std::string> tuple_types;      // Tuple variant: Variant(Type1, Type2)
        std::vector<AstStructField> struct_fields; // Struct variant: Variant { field: Type }

        int line = 0;
        int column = 0;

        bool has_data() const { return !tuple_types.empty() || !struct_fields.empty(); }
        bool is_tuple_variant() const { return !tuple_types.empty(); }
        bool is_struct_variant() const { return !struct_fields.empty(); }
    };

    struct AstEnumDecl : AstDecl {
        std::string name;
        std::vector<AstEnumVariant> variants;
        bool is_pub = false;  // pub enum
        bool declared_as_variant = false;  // true if declared with 'variant' keyword

        bool is_public() const override { return is_pub; }

        // Check if this is an algebraic data type (has variants with data)
        bool has_data_variants() const {
            for (const auto& v : variants) {
                if (v.has_data()) return true;
            }
            return false;
        }

        explicit AstEnumDecl(std::string n, int line = 0, int column = 0)
            : AstDecl(NodeKind::EnumDecl, line, column),
            name(std::move(n)) {
        }
    };

    // Associated type declaration in a trait: type Item;
    struct AstAssociatedType {
        std::string name;
        int line = 0;
        int column = 0;
    };

    // Associated type assignment in impl: type Item = i32;
    struct AstTypeAssignment {
        std::string name;        // Associated type name (e.g., "Item")
        std::string target_type; // Concrete type (e.g., "i32")
        int line = 0;
        int column = 0;
    };

    // Trait method signature with optional default implementation
    struct AstTraitMethod {
        std::string name;
        std::vector<AstParam> params;
        std::string return_type;
        std::unique_ptr<AstBlockStmt> body;  // Optional default implementation
        bool takes_self = false;  // Whether first param is 'self'
        int line = 0;
        int column = 0;

        bool has_default() const { return body != nullptr; }
    };

    // trait Printable { fn print(self) -> void; }
    struct AstTraitDecl : AstDecl {
        std::string name;
        std::vector<AstAssociatedType> associated_types;  // type Item;
        std::vector<AstTraitMethod> methods;
        bool is_pub = false;  // pub trait

        bool is_public() const override { return is_pub; }

        explicit AstTraitDecl(std::string n, int line = 0, int column = 0)
            : AstDecl(NodeKind::TraitDecl, line, column),
            name(std::move(n)) {
        }
    };

    // const NAME: Type = value; in impl blocks
    struct AstImplConst {
        std::string name;
        std::string type_name;
        std::unique_ptr<AstExpr> init_expr;
        int line = 0;
        int column = 0;
    };

    // impl TraitName for TypeName { fn method(self) -> void { ... } }
    struct AstImplDecl : AstDecl {
        std::string trait_name;  // Empty for inherent impl (just methods on type)
        std::string type_name;
        std::vector<AstTypeAssignment> type_assignments;  // type Item = i32;
        std::vector<std::unique_ptr<AstFuncDecl>> methods;
        std::vector<AstImplConst> constants;  // const NAME: Type = value;

        AstImplDecl(int line = 0, int column = 0)
            : AstDecl(NodeKind::ImplDecl, line, column) {
        }

        bool is_trait_impl() const { return !trait_name.empty(); }
    };

    // type Alias = OriginalType;
    struct AstTypeAliasDecl : AstDecl {
        std::string alias_name;
        std::string target_type;
        bool is_pub = false;  // pub type

        bool is_public() const override { return is_pub; }

        AstTypeAliasDecl(std::string name, std::string target, int line = 0, int column = 0)
            : AstDecl(NodeKind::TypeAliasDecl, line, column),
              alias_name(std::move(name)),
              target_type(std::move(target)) {
        }
    };

} // namespace mana::frontend
