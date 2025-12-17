#include "Repl.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace mana::repl {

using namespace frontend;

Repl::Repl() {}

void Repl::print_banner() {
    std::cout << R"(
    _    ____ _____ ____      _
   / \  / ___|_   _|  _ \    / \
  / _ \ \___ \ | | | |_) |  / _ \
 / ___ \ ___) || | |  _ <  / ___ \
/_/   \_\____/ |_| |_| \_\/_/   \_\

)" << "Mana REPL v0.1.0\n";
    std::cout << "Type :help for help, :quit to exit\n\n";
}

void Repl::print_help() {
    std::cout << "Commands:\n";
    std::cout << "  :help, :h     - Show this help\n";
    std::cout << "  :quit, :q     - Exit REPL\n";
    std::cout << "  :clear, :c    - Clear screen\n";
    std::cout << "  :history      - Show command history\n";
    std::cout << "  :reset        - Reset all definitions\n";
    std::cout << "  :load <file>  - Load and execute a file\n";
    std::cout << "  :type <expr>  - Show type of expression\n";
    std::cout << "\n";
    std::cout << "Enter Mana expressions or statements to evaluate.\n";
    std::cout << "Multi-line input: end line with \\ to continue.\n";
}

std::string Repl::read_input() {
    std::string line;
    std::string input;
    bool continuation = false;

    while (true) {
        if (continuation) {
            std::cout << "... ";
        } else {
            std::cout << ">>> ";
        }
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            running_ = false;
            return "";
        }

        // Check for line continuation
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
            input += line + "\n";
            continuation = true;
            continue;
        }

        input += line;

        // Check for balanced braces
        int brace_count = 0;
        for (char c : input) {
            if (c == '{') brace_count++;
            else if (c == '}') brace_count--;
        }

        if (brace_count > 0) {
            input += "\n";
            continuation = true;
            continue;
        }

        break;
    }

    if (!input.empty()) {
        history_.push_back(input);
        history_index_ = history_.size();
    }

    return input;
}

void Repl::run() {
    print_banner();

    while (running_) {
        std::string input = read_input();
        if (input.empty()) continue;

        // Handle commands
        if (input[0] == ':') {
            if (input == ":quit" || input == ":q") {
                running_ = false;
            } else if (input == ":help" || input == ":h") {
                print_help();
            } else if (input == ":clear" || input == ":c") {
                #ifdef _WIN32
                std::system("cls");
                #else
                std::system("clear");
                #endif
            } else if (input == ":history") {
                for (size_t i = 0; i < history_.size(); i++) {
                    std::cout << i + 1 << ": " << history_[i] << "\n";
                }
            } else if (input == ":reset") {
                definitions_.clear();
                variables_.clear();
                std::cout << "Reset complete.\n";
            } else if (input.substr(0, 6) == ":load ") {
                std::string filename = input.substr(6);
                std::ifstream file(filename);
                if (file) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    execute(buffer.str());
                } else {
                    std::cerr << "Error: Could not open file '" << filename << "'\n";
                }
            } else if (input.substr(0, 6) == ":type ") {
                std::string expr = input.substr(6);
                // TODO: Implement type inference display
                std::cout << "Type checking not yet implemented\n";
            } else {
                std::cerr << "Unknown command: " << input << "\n";
            }
        } else {
            execute(input);
        }
    }

    std::cout << "Goodbye!\n";
}

void Repl::execute(const std::string& input) {
    // Wrap input for parsing
    std::string code = "module repl;\n" + definitions_ + "\nfn __repl_main() -> void {\n" + input + "\n}\n";

    // Parse
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    DiagnosticEngine diag;
    Parser parser(tokens, diag);
    auto module = parser.parse_module();

    // Check for errors
    if (!diag.errors().empty()) {
        for (const auto& err : diag.errors()) {
            // Adjust line numbers (subtract header lines)
            int adjusted_line = err.line - 2 - static_cast<int>(std::count(definitions_.begin(), definitions_.end(), '\n'));
            if (adjusted_line < 1) adjusted_line = 1;
            std::cerr << "error: " << err.message << " (line " << adjusted_line << ")\n";
        }
        return;
    }

    // Semantic analysis
    DiagnosticEngine sem_diag;
    SemanticAnalyzer analyzer(sem_diag);
    analyzer.analyze(module.get());

    if (!sem_diag.errors().empty()) {
        for (const auto& err : sem_diag.errors()) {
            int adjusted_line = err.line - 2 - static_cast<int>(std::count(definitions_.begin(), definitions_.end(), '\n'));
            if (adjusted_line < 1) adjusted_line = 1;
            std::cerr << "error: " << err.message << " (line " << adjusted_line << ")\n";
        }
        return;
    }

    for (const auto& warn : sem_diag.warnings()) {
        int adjusted_line = warn.line - 2 - static_cast<int>(std::count(definitions_.begin(), definitions_.end(), '\n'));
        if (adjusted_line < 1) adjusted_line = 1;
        std::cerr << "warning: " << warn.message << " (line " << adjusted_line << ")\n";
    }

    // If input looks like a definition, save it
    if (input.find("fn ") == 0 || input.find("struct ") == 0 ||
        input.find("enum ") == 0 || input.find("trait ") == 0 ||
        input.find("impl ") == 0 || input.find("pub ") == 0) {
        definitions_ += input + "\n";
        std::cout << "Defined.\n";
    } else {
        std::cout << "OK\n";
    }
}

} // namespace mana::repl
