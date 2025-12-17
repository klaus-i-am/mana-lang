#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "../frontend/Lexer.h"
#include "../frontend/Parser.h"
#include "../frontend/Semantic.h"
#include "../backend-cpp/CppEmitter.h"

namespace mana {

inline int run_repl(const char* runtime_header) {
    namespace fs = std::filesystem;
    using namespace mana::frontend;
    using namespace mana::backend;

    std::cout << "Mana REPL v0.1\n";
    std::cout << "Type expressions to evaluate, 'exit' or 'quit' to exit.\n\n";

    std::string line;
    int expr_count = 0;

    // Create a persistent temp directory for REPL
    fs::path temp_dir = fs::temp_directory_path() / "mana_repl";
    fs::create_directories(temp_dir);

    // Write runtime header once
    {
        std::ofstream runtime_out(temp_dir / "mana_runtime.h");
        runtime_out << runtime_header;
    }

    // Create CMakeLists.txt once
    {
        std::ofstream cmake_out(temp_dir / "CMakeLists.txt");
        cmake_out << "cmake_minimum_required(VERSION 3.16)\n";
        cmake_out << "project(mana_repl)\n";
        cmake_out << "set(CMAKE_CXX_STANDARD 20)\n";
        cmake_out << "add_executable(repl_eval repl_eval.cpp)\n";
    }

    // Configure cmake once
    fs::path build_dir = temp_dir / "build";
#ifdef _WIN32
    std::string cmake_config = "cmake -B \"" + build_dir.string() + "\" -S \"" + temp_dir.string() + "\" >nul 2>&1";
#else
    std::string cmake_config = "cmake -B " + build_dir.string() + " -S " + temp_dir.string() + " >/dev/null 2>&1";
#endif
    std::system(cmake_config.c_str());

    while (true) {
        std::cout << ">>> ";
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start, end - start + 1);

        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        // Create a temporary program wrapping the expression
        std::string source;
        bool is_let = line.rfind("let ", 0) == 0;
        bool is_fn = line.rfind("fn ", 0) == 0;

        if (is_fn || is_let) {
            source = "module repl;\n" + line + "\nfn main() -> i32 { return 0; }";
        } else {
            source = "module repl;\nfn main() -> i32 { println(" + line + "); return 0; }";
        }

        DiagnosticEngine diag;
        diag.set_source("<repl>", source);

        Lexer lex(source);
        auto tokens = lex.tokenize();

        Parser parser(tokens, diag);
        auto module = parser.parse_module();

        if (!module || diag.has_errors()) {
            diag.print_all(std::cerr);
            continue;
        }

        SemanticAnalyzer sema(diag);
        sema.analyze(module.get());

        if (diag.has_errors()) {
            diag.print_all(std::cerr);
            continue;
        }

        // Generate C++ code
        std::ostringstream cpp_stream;
        CppEmitter emit;
        emit.emit(module.get(), cpp_stream, false);
        std::string cpp_code = cpp_stream.str();

        // Write generated C++
        fs::path cpp_file = temp_dir / "repl_eval.cpp";
        {
            std::ofstream cpp_out(cpp_file);
            cpp_out << cpp_code;
        }

        // Build with cmake
#ifdef _WIN32
        std::string cmake_build = "cmake --build \"" + build_dir.string() + "\" --config Release >nul 2>&1";
        fs::path exe_file = build_dir / "Release" / "repl_eval.exe";
#else
        std::string cmake_build = "cmake --build " + build_dir.string() + " >/dev/null 2>&1";
        fs::path exe_file = build_dir / "repl_eval";
#endif

        int result = std::system(cmake_build.c_str());
        if (result != 0) {
            std::cerr << "Compilation error\n";
            continue;
        }

        // Run the executable
        std::string run_cmd = "\"" + exe_file.string() + "\"";
        std::system(run_cmd.c_str());

        expr_count++;
    }

    return 0;
}

} // namespace mana
