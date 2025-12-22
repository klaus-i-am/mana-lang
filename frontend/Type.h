#pragma once
#include <string>

namespace mana::frontend {

    enum class TypeKind {
        Void,
        I32,
        F32,
        Bool,
        String,
        Struct,
        Enum,
        Array,
        Tuple,          // (T1, T2, ...) - tuple type
        Pointer,
        Reference,      // &T - immutable reference
        MutReference,   // &mut T - mutable reference
        Function,       // fn(T1, T2) -> R - function/closure type
        Unknown
    };

    struct Type {
        TypeKind kind = TypeKind::Unknown;
        std::string struct_name;  // Also used for enum name
        std::string element_type; // For arrays, pointers, references
        int array_size = 0;       // For fixed-size arrays (0 = dynamic)
        std::string original_name; // Preserves the original type name (e.g., "i64" even when kind is I32)

        static Type i32() { Type t; t.kind = TypeKind::I32; t.original_name = "i32"; return t; }
        static Type i64() { Type t; t.kind = TypeKind::I32; t.original_name = "i64"; return t; }
        static Type f32() { Type t; t.kind = TypeKind::F32; t.original_name = "f32"; return t; }
        static Type f64() { Type t; t.kind = TypeKind::F32; t.original_name = "f64"; return t; }
        static Type boolean() { return { TypeKind::Bool }; }
        static Type string() { return { TypeKind::String }; }
        static Type void_() { return { TypeKind::Void }; }
        static Type unknown() { return { TypeKind::Unknown }; }
        static Type struct_(const std::string& name) { return { TypeKind::Struct, name }; }
        static Type enum_(const std::string& name) { return { TypeKind::Enum, name }; }
        static Type array(const std::string& elem, int size) {
            Type t;
            t.kind = TypeKind::Array;
            t.element_type = elem;
            t.array_size = size;
            return t;
        }
        static Type pointer(const std::string& pointee) {
            Type t;
            t.kind = TypeKind::Pointer;
            t.element_type = pointee;
            return t;
        }
        static Type reference(const std::string& referent) {
            Type t;
            t.kind = TypeKind::Reference;
            t.element_type = referent;
            return t;
        }
        static Type mut_reference(const std::string& referent) {
            Type t;
            t.kind = TypeKind::MutReference;
            t.element_type = referent;
            return t;
        }
        // Tuple type: stores element types as "(T1, T2, ...)" in struct_name
        static Type tuple(const std::string& elements) {
            Type t;
            t.kind = TypeKind::Tuple;
            t.struct_name = elements;  // e.g., "(i32, string, bool)"
            return t;
        }
        // Function type: stores signature as "fn(T1, T2) -> R" in struct_name
        // element_type stores the return type name
        static Type function(const std::string& param_types, const std::string& return_type) {
            Type t;
            t.kind = TypeKind::Function;
            t.struct_name = "fn(" + param_types + ") -> " + return_type;
            t.element_type = return_type;  // Store return type for easy access
            return t;
        }

        bool operator==(const Type& o) const {
            if (kind != o.kind) return false;
            if (kind == TypeKind::Struct || kind == TypeKind::Enum || kind == TypeKind::Tuple)
                return struct_name == o.struct_name;
            if (kind == TypeKind::Array)
                return element_type == o.element_type && array_size == o.array_size;
            if (kind == TypeKind::Pointer || kind == TypeKind::Reference || kind == TypeKind::MutReference)
                return element_type == o.element_type;
            if (kind == TypeKind::Function)
                return struct_name == o.struct_name;
            return true;
        }
        bool operator!=(const Type& o) const { return !(*this == o); }

        std::string name() const {
            switch (kind) {
            case TypeKind::Void: return "void";
            case TypeKind::I32: return original_name.empty() ? "i32" : original_name;
            case TypeKind::F32: return original_name.empty() ? "f32" : original_name;
            case TypeKind::Bool: return "bool";
            case TypeKind::String: return "string";
            case TypeKind::Struct: return struct_name;
            case TypeKind::Enum: return struct_name;
            case TypeKind::Array:
                return "[" + std::to_string(array_size) + "]" + element_type;
            case TypeKind::Tuple:
                return struct_name;  // Already in "(T1, T2, ...)" format
            case TypeKind::Pointer:
                return "*" + element_type;
            case TypeKind::Reference:
                return "&" + element_type;
            case TypeKind::MutReference:
                return "&mut " + element_type;
            case TypeKind::Function:
                return struct_name;
            case TypeKind::Unknown:
                // Return struct_name if set (for dyn types, etc.)
                if (!struct_name.empty()) return struct_name;
                return "<unknown>";
            default: return "<unknown>";
            }
        }
    };

} // namespace mana::frontend
