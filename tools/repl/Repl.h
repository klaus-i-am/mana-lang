#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../../frontend/Parser.h"
#include "../../frontend/Semantic.h"
#include "../../frontend/Diagnostic.h"

namespace mana::repl {

    class Repl {
    public:
        Repl();
        void run();

    private:
        void print_banner();
        void print_help();
        std::string read_input();
        void execute(const std::string& input);
        void execute_expression(const std::string& input);
        void execute_statement(const std::string& input);

        // History
        std::vector<std::string> history_;
        size_t history_index_ = 0;

        // Variables in scope
        std::unordered_map<std::string, std::string> variables_;

        // Accumulated definitions (functions, structs, etc.)
        std::string definitions_;

        bool running_ = true;
    };

} // namespace mana::repl
