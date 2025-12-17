0) Context / Goal

We are building Mana, a readable C++-inspired language aimed at game-engine development. The compiler is written in C++ (Visual Studio 2022 / Win11) and currently compiles .mana to a generated C++ file (RAII lowering + control flow supported). The project has repeatedly “exploded” due to header/AST architecture drift (types renamed, split inconsistently, circular includes, different “AstNode/AstExpr/AstStmt” hierarchies used in different parts).

Rule: Do not invent new AST base types or rename them unless we update every single file consistently.

1) Current Working State (known-good snapshot)

At the last confirmed working point:

Compiler builds.

It prints AST.

It performs semantic analysis with types.

It emits C++ code that correctly handles:

if / else

while

break / continue

defer (lowered to RAII helper)

assignment statements

The output C++ looked correct for while/if/break/continue.

The project then entered a broken state when we attempted for loops and refactors.

IMPORTANT: The user’s codebase currently has an AST setup where expressions live in AstDeclarations.h (misnamed but accepted), and there is AstNodes.h that defines AstNode and NodeKind. A huge class of errors came from accidentally switching to a different AST layout (like “AstModule.h / AstStatements.h / AstExpressions.h” or introducing “AstExpr” in a different file name) without synchronizing everything.

2) Non-negotiable style for responses

When implementing features, provide FULL FILES (not snippets) with correct includes and matching declarations/definitions. The user pastes whole files. No “merge this” partial patches.

3) Toolchain / Platform

Windows 11

Visual Studio 2022

Multi-project or single solution, but the compiler executable is mana-compiler.

Input usually is ../tests/sample.mana

Output is ../out/generated.cpp

The working main.cpp reads file, lexes, parses, sema checks, prints AST, emits C++.

4) The Architecture Issue That Caused “Errors Exploded”
Root cause pattern:

A header was renamed or replaced (e.g., “AstModule.h” vs “AstNodes.h”).

A base type changed (AstNode vs AstExpr vs AstStmt) but not updated everywhere.

NodeKind enum drifted (some code used old NodeKind labels).

Includes were inconsistent: a file needed #include "AstModule.h" or AstDeclarations.h but only had forward decls.

Circular includes caused “use of undefined type”, then cascaded into 100+ errors.

Prevent it:

Keep one canonical AST layout.

Keep NodeKind in one file and include it consistently.

Use forward declarations only when safe; otherwise include the header.

5) Canonical AST Layout (as currently in user project)
AstNodes.h

Defines enum class NodeKind { ... }

Defines struct AstNode { NodeKind kind; ... }

Likely includes line and column or not (in previous working version, many nodes had line/column fields; later errors showed missing, so this must be consistent across the codegen/sema/printer).

AstDeclarations.h (user’s current content)

Despite the name, this file currently defines expressions:

#pragma once
#include <memory>
#include <string>
#include <vector>
#include "AstNodes.h"

namespace mana::frontend {

    struct AstExpr : AstNode {
        explicit AstExpr(NodeKind k) : AstNode(k) {}
        virtual ~AstExpr() = default;
    };

    struct AstIdentifierExpr : AstExpr { std::string name; ... };
    struct AstLiteralExpr : AstExpr { std::string value; ... };

    struct AstCallExpr : AstExpr {
        std::string func_name;
        std::vector<std::unique_ptr<AstNode>> args;
        ...
    };

    struct AstBinaryExpr : AstExpr {
        std::string op;
        std::unique_ptr<AstNode> left;
        std::unique_ptr<AstNode> right;
        ...
    };
}


Important: args/left/right are unique_ptr<AstNode> not AstExpr. Do not change that unless we refactor EVERYTHING.

Other AST files exist too (statements, module, etc.)

There are also statement nodes such as:

BlockStmt

VarDeclStmt

AssignStmt

ScopeStmt

DeferStmt

ReturnStmt

ExprStmt

IfStmt

WhileStmt

BreakStmt

ContinueStmt

And declarations:

FuncDecl (has name, return_type, params, body)

GlobalVarDecl (wraps a VarDeclStmt or similar)

Import (maybe)

But file names and exact structs must match the user repo.
When implementing new features, Claude must ask the user to paste their current headers if uncertain (BUT prefer best-effort: keep consistent with what exists).

6) Semantic Analyzer

Has symbol table with scopes vector.

Has builtin function load_texture typed as string.

Does type checking for:

i32, f32, string, bool, void

return type checking

call argument count/type checking when func decl known

binary operator type rules (currently “operands same type”, unary uses left==null trick or separate unary node depending on snapshot)

loop depth checks for break/continue (added in working version)

Diagnostics engine supports error(msg, line, col) and warning(msg, line, col) and has_errors() and has_any().

7) Codegen (C++ emitter)

Emits // Generated by mana-compiler

Includes mana_runtime.h

Emits functions

Emits while/if/break/continue/defer

Defer is lowered to RAII: auto __mana_defer_N = ::mana::defer([&]() { ... });

Assign statement emits x = ...;

VarDecl emits type name = init; (C++ types mapped from Mana types)

8) What “RESET / Option A” meant

Option A = stabilize architecture first:

Stop adding features until headers and AST hierarchy are consistent again.

Ensure include paths and naming are correct.

Ensure Parser.h and Parser.cpp match exactly.

Ensure Lexer.h and Lexer.cpp match exactly.

9) What to do next (planned features)

Immediate next feature originally: for loops.
But because the architecture kept breaking, we should:

Confirm the project builds and runs for a known sample.

Only then add for loop as a desugaring pass:

Parse for init; cond; step { body }

Transform into:

{ init; while cond { body; step; } }

Keep parser minimal; avoid major AST expansion if possible.

Also optional: i++ support.

This is sugar for i = i + 1.

Should be implemented as token ++ + parse as postfix increment OR just keep it out until core stable.

10) Critical Instruction to Claude

When continuing:

Do not invent new file names.

Do not rename AstDeclarations.h even if misnamed.

Provide FULL FILES for every changed file.

Keep AstNode as the universal pointer type in the AST unless the user already refactored.

If adding a new node kind (ForStmt), update:

NodeKind enum

Parser

AstPrinter

SemanticAnalyzer

Codegen emitter

Any desugar pass (if used)
All in the same answer, full files.

11) Known Debugging Tricks (Windows/VS)

Working directory confusion: using relative paths depends on the project’s Debugging “Working Directory”.

Use ../tests/sample.mana when executable runs from mana-compiler/.

Output ../out/generated.cpp requires out/ folder to exist or creation.

12) Current Pain Point

The user keeps getting errors like “AstExpr undefined”, “cannot open AstModule.h”, “AstNode base undefined”, “struct redefinition”, etc.
These are almost always include mismatch / double-definition / different AST versions compiled together.

Fix strategy:

Ensure there is only ONE set of AST headers in include path.

Check that VS project includes correct folders and excludes old duplicates.

Ensure include guards / #pragma once present.

Remove stale duplicate headers from project filters if they exist.