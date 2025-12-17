#include "LspServer.h"
#include <regex>
#include <algorithm>

namespace mana::lsp {

LspServer::LspServer() {}

void LspServer::run() {
    while (running_) {
        std::string msg = read_message();
        if (!msg.empty()) {
            handle_message(msg);
        }
    }
}

std::string LspServer::read_message() {
    // Read headers until empty line
    std::string headers;
    std::string line;
    int content_length = 0;

    while (std::getline(std::cin, line)) {
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) break;

        if (line.find("Content-Length:") == 0) {
            content_length = std::stoi(line.substr(15));
        }
    }

    if (content_length == 0) return "";

    // Read content
    std::string content(content_length, '\0');
    std::cin.read(&content[0], content_length);
    return content;
}

void LspServer::write_message(const std::string& content) {
    std::cout << "Content-Length: " << content.size() << "\r\n\r\n" << content;
    std::cout.flush();
}

void LspServer::handle_message(const std::string& json) {
    // Extract method and id
    std::string method = extract_string(json, "method");
    int id = extract_int(json, "id");

    // Find params section
    size_t params_pos = json.find("\"params\"");
    std::string params = "{}";
    if (params_pos != std::string::npos) {
        size_t start = json.find('{', params_pos);
        if (start != std::string::npos) {
            int depth = 1;
            size_t end = start + 1;
            while (end < json.size() && depth > 0) {
                if (json[end] == '{') depth++;
                else if (json[end] == '}') depth--;
                end++;
            }
            params = json.substr(start, end - start);
        }
    }

    if (method == "initialize") {
        handle_initialize(id, params);
    } else if (method == "initialized") {
        handle_initialized();
    } else if (method == "shutdown") {
        handle_shutdown(id);
    } else if (method == "exit") {
        handle_exit();
    } else if (method == "textDocument/didOpen") {
        handle_text_document_did_open(params);
    } else if (method == "textDocument/didChange") {
        handle_text_document_did_change(params);
    } else if (method == "textDocument/didClose") {
        handle_text_document_did_close(params);
    } else if (method == "textDocument/hover") {
        handle_text_document_hover(id, params);
    } else if (method == "textDocument/completion") {
        handle_text_document_completion(id, params);
    } else if (method == "textDocument/definition") {
        handle_text_document_definition(id, params);
    }
}

void LspServer::handle_initialize(int id, const std::string& params) {
    std::string response = R"({
        "jsonrpc": "2.0",
        "id": )" + std::to_string(id) + R"(,
        "result": {
            "capabilities": {
                "textDocumentSync": 1,
                "hoverProvider": true,
                "completionProvider": {
                    "triggerCharacters": [".", ":", "<"]
                },
                "definitionProvider": true
            },
            "serverInfo": {
                "name": "mana-lsp",
                "version": "0.1.0"
            }
        }
    })";
    write_message(response);
}

void LspServer::handle_initialized() {
    initialized_ = true;
}

void LspServer::handle_shutdown(int id) {
    std::string response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": null})";
    write_message(response);
}

void LspServer::handle_exit() {
    running_ = false;
}

void LspServer::handle_text_document_did_open(const std::string& params) {
    // Extract URI and text from textDocument
    size_t uri_pos = params.find("\"uri\"");
    std::string uri = extract_string(params, "uri");

    size_t text_pos = params.find("\"text\"");
    if (text_pos != std::string::npos) {
        size_t start = params.find('"', text_pos + 6) + 1;
        size_t end = start;
        while (end < params.size()) {
            if (params[end] == '"' && params[end-1] != '\\') break;
            end++;
        }
        std::string text = params.substr(start, end - start);
        // Unescape
        std::string unescaped;
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\\' && i + 1 < text.size()) {
                switch (text[i+1]) {
                    case 'n': unescaped += '\n'; i++; break;
                    case 't': unescaped += '\t'; i++; break;
                    case 'r': unescaped += '\r'; i++; break;
                    case '"': unescaped += '"'; i++; break;
                    case '\\': unescaped += '\\'; i++; break;
                    default: unescaped += text[i]; break;
                }
            } else {
                unescaped += text[i];
            }
        }
        documents_[uri] = unescaped;
        analyze_document(uri);
    }
}

void LspServer::handle_text_document_did_change(const std::string& params) {
    std::string uri = extract_string(params, "uri");

    // Find contentChanges array and extract text
    size_t changes_pos = params.find("\"contentChanges\"");
    if (changes_pos != std::string::npos) {
        size_t text_pos = params.find("\"text\"", changes_pos);
        if (text_pos != std::string::npos) {
            size_t start = params.find('"', text_pos + 6) + 1;
            size_t end = start;
            while (end < params.size()) {
                if (params[end] == '"' && params[end-1] != '\\') break;
                end++;
            }
            std::string text = params.substr(start, end - start);
            // Unescape
            std::string unescaped;
            for (size_t i = 0; i < text.size(); i++) {
                if (text[i] == '\\' && i + 1 < text.size()) {
                    switch (text[i+1]) {
                        case 'n': unescaped += '\n'; i++; break;
                        case 't': unescaped += '\t'; i++; break;
                        case 'r': unescaped += '\r'; i++; break;
                        case '"': unescaped += '"'; i++; break;
                        case '\\': unescaped += '\\'; i++; break;
                        default: unescaped += text[i]; break;
                    }
                } else {
                    unescaped += text[i];
                }
            }
            documents_[uri] = unescaped;
            analyze_document(uri);
        }
    }
}

void LspServer::handle_text_document_did_close(const std::string& params) {
    std::string uri = extract_string(params, "uri");
    documents_.erase(uri);
    parsed_modules_.erase(uri);
    // Clear diagnostics
    publish_diagnostics(uri, {});
}

void LspServer::handle_text_document_hover(int id, const std::string& params) {
    std::string uri = extract_string(params, "uri");
    int line = extract_int(params, "line");
    int character = extract_int(params, "character");

    std::string info = get_hover_info(uri, {line, character});

    std::string response;
    if (info.empty()) {
        response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": null})";
    } else {
        response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": {"contents": {"kind": "markdown", "value": ")" + info + R"("}}})";
    }
    write_message(response);
}

void LspServer::handle_text_document_completion(int id, const std::string& params) {
    std::string uri = extract_string(params, "uri");
    int line = extract_int(params, "line");
    int character = extract_int(params, "character");

    auto items = get_completions(uri, {line, character});

    std::string items_json = "[";
    for (size_t i = 0; i < items.size(); i++) {
        if (i > 0) items_json += ",";
        items_json += R"({"label": ")" + items[i].label + R"(", "kind": )" + std::to_string(items[i].kind);
        if (!items[i].detail.empty()) {
            items_json += R"(, "detail": ")" + items[i].detail + R"(")";
        }
        items_json += "}";
    }
    items_json += "]";

    std::string response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": )" + items_json + "}";
    write_message(response);
}

void LspServer::handle_text_document_definition(int id, const std::string& params) {
    std::string uri = extract_string(params, "uri");
    int line = extract_int(params, "line");
    int character = extract_int(params, "character");

    Location loc = get_definition(uri, {line, character});

    std::string response;
    if (loc.uri.empty()) {
        response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": null})";
    } else {
        response = R"({"jsonrpc": "2.0", "id": )" + std::to_string(id) + R"(, "result": {"uri": ")" + loc.uri +
            R"(", "range": {"start": {"line": )" + std::to_string(loc.range.start.line) +
            R"(, "character": )" + std::to_string(loc.range.start.character) +
            R"(}, "end": {"line": )" + std::to_string(loc.range.end.line) +
            R"(, "character": )" + std::to_string(loc.range.end.character) + "}}}})";
    }
    write_message(response);
}

void LspServer::analyze_document(const std::string& uri) {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return;

    const std::string& content = it->second;
    std::vector<Diagnostic> diagnostics;

    // Create a diagnostic collector
    std::vector<std::pair<std::string, std::tuple<int, int, int, int>>> errors;

    // Parse the document
    frontend::Lexer lexer(content);
    auto tokens = lexer.tokenize();

    frontend::DiagnosticEngine diag;
    frontend::Parser parser(tokens, diag);
    auto module = parser.parse_module();

    // Collect parse errors
    for (const auto& err : diag.errors()) {
        Diagnostic d;
        d.range.start.line = err.line - 1;  // LSP is 0-indexed
        d.range.start.character = err.column - 1;
        d.range.end.line = err.line - 1;
        d.range.end.character = err.column + 10;
        d.severity = 1;  // Error
        d.message = err.message;
        diagnostics.push_back(d);
    }

    // Run semantic analysis if parsing succeeded
    if (module && diag.errors().empty()) {
        frontend::DiagnosticEngine sem_diag;
        frontend::SemanticAnalyzer analyzer(sem_diag);
        analyzer.analyze(module.get());

        for (const auto& err : sem_diag.errors()) {
            Diagnostic d;
            d.range.start.line = err.line - 1;
            d.range.start.character = err.column - 1;
            d.range.end.line = err.line - 1;
            d.range.end.character = err.column + 10;
            d.severity = 1;
            d.message = err.message;
            diagnostics.push_back(d);
        }

        for (const auto& warn : sem_diag.warnings()) {
            Diagnostic d;
            d.range.start.line = warn.line - 1;
            d.range.start.character = warn.column - 1;
            d.range.end.line = warn.line - 1;
            d.range.end.character = warn.column + 10;
            d.severity = 2;  // Warning
            d.message = warn.message;
            diagnostics.push_back(d);
        }

        parsed_modules_[uri] = std::move(module);
    }

    publish_diagnostics(uri, diagnostics);
}

void LspServer::publish_diagnostics(const std::string& uri, const std::vector<Diagnostic>& diagnostics) {
    std::string diags_json = "[";
    for (size_t i = 0; i < diagnostics.size(); i++) {
        if (i > 0) diags_json += ",";
        const auto& d = diagnostics[i];
        diags_json += R"({"range": {"start": {"line": )" + std::to_string(d.range.start.line) +
            R"(, "character": )" + std::to_string(d.range.start.character) +
            R"(}, "end": {"line": )" + std::to_string(d.range.end.line) +
            R"(, "character": )" + std::to_string(d.range.end.character) +
            R"(}}, "severity": )" + std::to_string(d.severity) +
            R"(, "source": "mana", "message": )" + json_string(d.message) + "}";
    }
    diags_json += "]";

    std::string notification = R"({"jsonrpc": "2.0", "method": "textDocument/publishDiagnostics", "params": {"uri": ")" +
        uri + R"(", "diagnostics": )" + diags_json + "}}";
    write_message(notification);
}

std::vector<CompletionItem> LspServer::get_completions(const std::string& uri, Position pos) {
    std::vector<CompletionItem> items;

    // Keywords
    static const std::vector<std::pair<std::string, std::string>> keywords = {
        {"fn", "Function declaration"}, {"let", "Variable declaration"}, {"mut", "Mutable variable"},
        {"if", "Conditional"}, {"else", "Else branch"}, {"match", "Pattern matching"},
        {"for", "For loop"}, {"while", "While loop"}, {"return", "Return statement"},
        {"struct", "Struct declaration"}, {"enum", "Enum declaration"}, {"trait", "Trait declaration"},
        {"impl", "Implementation block"}, {"pub", "Public visibility"}, {"use", "Import declaration"},
        {"async", "Async function"}, {"await", "Await expression"}, {"true", "Boolean true"},
        {"false", "Boolean false"}, {"None", "None value"}, {"Some", "Some wrapper"},
        {"Ok", "Ok result"}, {"Err", "Error result"}
    };

    for (const auto& [kw, desc] : keywords) {
        items.push_back({kw, 14, desc, ""});  // 14 = Keyword
    }

    // Types
    static const std::vector<std::string> types = {
        "i32", "i64", "f32", "f64", "bool", "string", "void", "Vec", "Option", "Result"
    };
    for (const auto& t : types) {
        items.push_back({t, 7, "Type", ""});  // 7 = Class
    }

    // Built-in functions
    static const std::vector<std::pair<std::string, std::string>> builtins = {
        {"println", "Print with newline"}, {"print", "Print without newline"},
        {"len", "Get length"}, {"push", "Push to collection"}, {"pop", "Pop from collection"}
    };
    for (const auto& [fn, desc] : builtins) {
        items.push_back({fn, 3, desc, ""});  // 3 = Function
    }

    return items;
}

std::string LspServer::get_hover_info(const std::string& uri, Position pos) {
    // TODO: Implement proper hover info based on AST
    return "";
}

Location LspServer::get_definition(const std::string& uri, Position pos) {
    // TODO: Implement proper go-to-definition based on AST
    return {};
}

std::string LspServer::json_string(const std::string& s) {
    std::string result = "\"";
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    result += "\"";
    return result;
}

std::string LspServer::extract_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    size_t colon = json.find(':', pos);
    if (colon == std::string::npos) return "";

    size_t start = json.find('"', colon) + 1;
    size_t end = start;
    while (end < json.size() && !(json[end] == '"' && json[end-1] != '\\')) end++;
    return json.substr(start, end - start);
}

int LspServer::extract_int(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return -1;

    size_t colon = json.find(':', pos);
    if (colon == std::string::npos) return -1;

    size_t start = colon + 1;
    while (start < json.size() && (json[start] == ' ' || json[start] == '\t')) start++;

    std::string num;
    while (start < json.size() && (isdigit(json[start]) || json[start] == '-')) {
        num += json[start++];
    }
    return num.empty() ? -1 : std::stoi(num);
}

} // namespace mana::lsp
