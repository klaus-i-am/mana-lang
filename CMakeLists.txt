cmake_minimum_required(VERSION 3.14)
project(mana_lang)

set(CMAKE_CXX_STANDARD 17)

include_directories(backend-cpp)
include_directories(frontend)
include_directories(middle)
include_directories(tools)

# Frontend library (shared between compiler and tools)
add_library(mana_frontend STATIC
        frontend/Ast.cpp
        frontend/Diagnostic.cpp
        frontend/Lexer.cpp
        frontend/Parser.cpp
        frontend/Semantic.cpp
        frontend/Token.cpp
        frontend/AstPrinter.cpp)

# Main compiler executable
add_executable(mana_lang
        backend-cpp/CppEmitter.cpp
        backend-cpp/DocGenerator.cpp
        core/main.cpp
        middle/ForLowering.cpp
        middle/DeadCodeElimination.cpp
        middle/Inlining.cpp
        tools/fmt/Formatter.cpp
        tools/repl/Repl.cpp
        tools/pkg/PackageManager.cpp
        tools/debug/Debugger.cpp
        tools/test/TestRunner.cpp)

target_link_libraries(mana_lang mana_frontend)

# LSP Server (separate executable)
add_executable(mana-lsp
        tools/lsp/main.cpp
        tools/lsp/LspServer.cpp)

target_link_libraries(mana-lsp mana_frontend)

# Debug adapter (separate executable for IDE integration)
add_executable(mana-debug
        tools/debug/main.cpp
        tools/debug/Debugger.cpp)

target_link_libraries(mana-debug mana_frontend)
