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
    auto doc_it = documents_.find(uri);
    if (doc_it == documents_.end()) return "";

    // Get the word at the cursor position
    const std::string& content = doc_it->second;
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    if (pos.line < 0 || pos.line >= static_cast<int>(lines.size())) return "";
    const std::string& current_line = lines[pos.line];
    if (pos.character < 0 || pos.character >= static_cast<int>(current_line.size())) return "";

    // Find word boundaries
    int start = pos.character;
    int end = pos.character;
    while (start > 0 && (std::isalnum(current_line[start - 1]) || current_line[start - 1] == '_')) start--;
    while (end < static_cast<int>(current_line.size()) && (std::isalnum(current_line[end]) || current_line[end] == '_')) end++;

    if (start == end) return "";
    std::string word = current_line.substr(start, end - start);

    // Check if we have a parsed module
    auto mod_it = parsed_modules_.find(uri);
    if (mod_it != parsed_modules_.end() && mod_it->second) {
        auto* module = mod_it->second.get();

        // Search for functions
        for (const auto& decl : module->decls) {
            if (auto* fn = dynamic_cast<frontend::AstFuncDecl*>(decl.get())) {
                if (fn->name == word) {
                    std::string info = "```mana\\nfn " + fn->name + "(";
                    for (size_t i = 0; i < fn->params.size(); ++i) {
                        if (i > 0) info += ", ";
                        info += fn->params[i].name + ": " + fn->params[i].type_name;
                    }
                    info += ") -> " + (fn->return_type.empty() ? "void" : fn->return_type);
                    info += "\\n```";
                    return info;
                }
            }
            // Search for structs
            if (auto* st = dynamic_cast<frontend::AstStructDecl*>(decl.get())) {
                if (st->name == word) {
                    std::string info = "```mana\\nstruct " + st->name;
                    if (!st->type_params.empty()) {
                        info += "<";
                        for (size_t i = 0; i < st->type_params.size(); ++i) {
                            if (i > 0) info += ", ";
                            info += st->type_params[i];
                        }
                        info += ">";
                    }
                    info += " {\\n";
                    for (const auto& field : st->fields) {
                        info += "    " + field.name + ": " + field.type_name + ",\\n";
                    }
                    info += "}\\n```";
                    return info;
                }
            }
            // Search for enums
            if (auto* en = dynamic_cast<frontend::AstEnumDecl*>(decl.get())) {
                if (en->name == word) {
                    std::string info = "```mana\\nenum " + en->name + " {\\n";
                    for (const auto& variant : en->variants) {
                        info += "    " + variant.name + ",\\n";
                    }
                    info += "}\\n```";
                    return info;
                }
            }
        }
    }

    // Check built-in types
    static const std::unordered_map<std::string, std::string> builtin_types = {
        {"i32", "32-bit signed integer"},
        {"i64", "64-bit signed integer"},
        {"f32", "32-bit floating point"},
        {"f64", "64-bit floating point"},
        {"bool", "Boolean type (true/false)"},
        {"string", "UTF-8 string type"},
        {"void", "No return value"},
        {"Vec", "Dynamic array collection"},
        {"Option", "Optional value: Some(T) or None"},
        {"Result", "Result type: Ok(T) or Err(E)"},
        {"HashMap", "Key-value hash map"},
    };

    auto type_it = builtin_types.find(word);
    if (type_it != builtin_types.end()) {
        return "**" + word + "**\\n\\n" + type_it->second;
    }

    // Check built-in functions
    static const std::unordered_map<std::string, std::string> builtin_fns = {
        {"println", "```mana\\nfn println(value: any) -> void\\n```\\n\\nPrints value with newline"},
        {"print", "```mana\\nfn print(value: any) -> void\\n```\\n\\nPrints value without newline"},
        {"len", "```mana\\nfn len(s: string) -> i32\\n```\\n\\nReturns length of string or collection"},
        {"push", "```mana\\nfn push(vec: Vec<T>, value: T) -> void\\n```\\n\\nAppends value to vector"},
        {"pop", "```mana\\nfn pop(vec: Vec<T>) -> Option<T>\\n```\\n\\nRemoves and returns last element"},
        {"Some", "```mana\\nfn Some<T>(value: T) -> Option<T>\\n```\\n\\nWraps value in Some variant"},
        {"None", "```mana\\nNone: Option<T>\\n```\\n\\nRepresents absence of value"},
        {"Ok", "```mana\\nfn Ok<T>(value: T) -> Result<T, E>\\n```\\n\\nSuccess result variant"},
        {"Err", "```mana\\nfn Err<E>(error: E) -> Result<T, E>\\n```\\n\\nError result variant"},
        {"sqrt", "```mana\\nfn sqrt(x: f32) -> f32\\n```\\n\\nSquare root"},
        {"sin", "```mana\\nfn sin(x: f32) -> f32\\n```\\n\\nSine (radians)"},
        {"cos", "```mana\\nfn cos(x: f32) -> f32\\n```\\n\\nCosine (radians)"},
        {"random_int", "```mana\\nfn random_int(min: i32, max: i32) -> i32\\n```\\n\\nRandom integer in range"},
        {"random_float", "```mana\\nfn random_float() -> f32\\n```\\n\\nRandom float 0.0 to 1.0"},
    };

    auto fn_it = builtin_fns.find(word);
    if (fn_it != builtin_fns.end()) {
        return fn_it->second;
    }

    // Check keywords
    static const std::unordered_map<std::string, std::string> keywords = {
        {"fn", "**fn**\\n\\nFunction declaration keyword"},
        {"let", "**let**\\n\\nVariable declaration (immutable by default)"},
        {"mut", "**mut**\\n\\nMutable variable modifier"},
        {"if", "**if**\\n\\nConditional statement"},
        {"else", "**else**\\n\\nAlternative branch"},
        {"match", "**match**\\n\\nPattern matching expression"},
        {"for", "**for**\\n\\nFor loop (for x in collection)"},
        {"while", "**while**\\n\\nWhile loop"},
        {"return", "**return**\\n\\nReturn from function"},
        {"struct", "**struct**\\n\\nDefine a structure type"},
        {"enum", "**enum**\\n\\nDefine an enumeration type"},
        {"impl", "**impl**\\n\\nImplementation block for methods"},
        {"trait", "**trait**\\n\\nDefine a trait (interface)"},
        {"pub", "**pub**\\n\\nPublic visibility modifier"},
        {"use", "**use**\\n\\nImport declaration"},
        {"async", "**async**\\n\\nAsynchronous function modifier"},
        {"await", "**await**\\n\\nAwait async operation"},
    };

    auto kw_it = keywords.find(word);
    if (kw_it != keywords.end()) {
        return kw_it->second;
    }

    return "";
}

Location LspServer::get_definition(const std::string& uri, Position pos) {
    auto doc_it = documents_.find(uri);
    if (doc_it == documents_.end()) return {};

    // Get the word at the cursor position
    const std::string& content = doc_it->second;
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    if (pos.line < 0 || pos.line >= static_cast<int>(lines.size())) return {};
    const std::string& current_line = lines[pos.line];
    if (pos.character < 0 || pos.character >= static_cast<int>(current_line.size())) return {};

    // Find word boundaries
    int start = pos.character;
    int end = pos.character;
    while (start > 0 && (std::isalnum(current_line[start - 1]) || current_line[start - 1] == '_')) start--;
    while (end < static_cast<int>(current_line.size()) && (std::isalnum(current_line[end]) || current_line[end] == '_')) end++;

    if (start == end) return {};
    std::string word = current_line.substr(start, end - start);

    // Check if we have a parsed module
    auto mod_it = parsed_modules_.find(uri);
    if (mod_it == parsed_modules_.end() || !mod_it->second) return {};

    auto* module = mod_it->second.get();

    // Search for functions
    for (const auto& decl : module->decls) {
        if (auto* fn = dynamic_cast<frontend::AstFuncDecl*>(decl.get())) {
            if (fn->name == word) {
                Location loc;
                loc.uri = uri;
                loc.range.start.line = fn->line - 1;  // LSP is 0-indexed
                loc.range.start.character = fn->column - 1;
                loc.range.end.line = fn->line - 1;
                loc.range.end.character = fn->column - 1 + static_cast<int>(fn->name.length());
                return loc;
            }
        }
        // Search for structs
        if (auto* st = dynamic_cast<frontend::AstStructDecl*>(decl.get())) {
            if (st->name == word) {
                Location loc;
                loc.uri = uri;
                loc.range.start.line = st->line - 1;
                loc.range.start.character = st->column - 1;
                loc.range.end.line = st->line - 1;
                loc.range.end.character = st->column - 1 + static_cast<int>(st->name.length());
                return loc;
            }
        }
        // Search for enums
        if (auto* en = dynamic_cast<frontend::AstEnumDecl*>(decl.get())) {
            if (en->name == word) {
                Location loc;
                loc.uri = uri;
                loc.range.start.line = en->line - 1;
                loc.range.start.character = en->column - 1;
                loc.range.end.line = en->line - 1;
                loc.range.end.character = en->column - 1 + static_cast<int>(en->name.length());
                return loc;
            }
        }
    }

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
