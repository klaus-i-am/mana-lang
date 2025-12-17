#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include "../../frontend/Parser.h"
#include "../../frontend/Semantic.h"
#include "../../frontend/Diagnostic.h"

namespace mana::lsp {

    struct Position {
        int line = 0;
        int character = 0;
    };

    struct Range {
        Position start;
        Position end;
    };

    struct Location {
        std::string uri;
        Range range;
    };

    struct Diagnostic {
        Range range;
        int severity = 1;  // 1=Error, 2=Warning, 3=Info, 4=Hint
        std::string message;
        std::string source = "mana";
    };

    struct CompletionItem {
        std::string label;
        int kind = 1;  // 1=Text, 2=Method, 3=Function, 6=Variable, 7=Class, 8=Interface, 22=Struct, 13=Enum
        std::string detail;
        std::string documentation;
    };

    class LspServer {
    public:
        LspServer();
        void run();

    private:
        // JSON-RPC handling
        std::string read_message();
        void write_message(const std::string& content);
        void handle_message(const std::string& json);

        // LSP methods
        void handle_initialize(int id, const std::string& params);
        void handle_initialized();
        void handle_shutdown(int id);
        void handle_exit();
        void handle_text_document_did_open(const std::string& params);
        void handle_text_document_did_change(const std::string& params);
        void handle_text_document_did_close(const std::string& params);
        void handle_text_document_hover(int id, const std::string& params);
        void handle_text_document_completion(int id, const std::string& params);
        void handle_text_document_definition(int id, const std::string& params);

        // Document management
        std::unordered_map<std::string, std::string> documents_;  // uri -> content
        std::unordered_map<std::string, std::unique_ptr<frontend::AstModule>> parsed_modules_;

        // Analyze and publish diagnostics
        void analyze_document(const std::string& uri);
        void publish_diagnostics(const std::string& uri, const std::vector<Diagnostic>& diagnostics);

        // Helpers
        std::vector<CompletionItem> get_completions(const std::string& uri, Position pos);
        std::string get_hover_info(const std::string& uri, Position pos);
        Location get_definition(const std::string& uri, Position pos);

        // Simple JSON helpers (basic implementation)
        static std::string json_string(const std::string& s);
        static std::string extract_string(const std::string& json, const std::string& key);
        static int extract_int(const std::string& json, const std::string& key);

        bool running_ = true;
        bool initialized_ = false;
    };

} // namespace mana::lsp
