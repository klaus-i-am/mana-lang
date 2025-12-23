#include "Debugger.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif

namespace mana {
namespace debug {

Debugger::Debugger()
    : m_initialized(false)
    , m_running(true)
    , m_terminated(false)
    , m_paused(false)
    , m_seq(0)
    , m_currentLine(1)
    , m_currentFrameId(1)
    , m_nextBreakpointId(1)
    , m_nextWatchId(1)
    , m_nextVariableRef(1)
    , m_stopOnEntry(true)
    , m_enableLogging(false)
#ifdef _WIN32
    , m_processHandle(nullptr)
    , m_threadHandle(nullptr)
    , m_processId(0)
    , m_debugEvent(nullptr)
#else
    , m_processPid(-1)
#endif
{
}

Debugger::~Debugger() {
    terminateProcess();
}

void Debugger::run() {
    while (m_running) {
        auto msg = readMessage();
        if (msg) {
            processMessage(*msg);
        } else {
            break;
        }
    }
}

std::optional<std::string> Debugger::readMessage() {
    // Read Content-Length header
    std::string header;
    while (true) {
        int c = std::cin.get();
        if (c == EOF) return std::nullopt;
        if (c == '\r') {
            c = std::cin.get(); // consume \n
            if (std::cin.peek() == '\r') {
                std::cin.get(); // consume \r
                std::cin.get(); // consume \n
                break;
            }
            header += '\n';
            continue;
        }
        header += static_cast<char>(c);
    }

    // Parse Content-Length
    size_t contentLength = 0;
    size_t pos = header.find("Content-Length:");
    if (pos != std::string::npos) {
        contentLength = std::stoul(header.substr(pos + 15));
    }

    if (contentLength == 0) return std::nullopt;

    // Read content
    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);

    return content;
}

void Debugger::writeMessage(const std::string& json) {
    std::cout << "Content-Length: " << json.length() << "\r\n\r\n" << json;
    std::cout.flush();
}

void Debugger::sendResponse(int requestSeq, const std::string& command,
                           bool success, const std::string& body) {
    std::ostringstream ss;
    ss << "{\"seq\":" << (++m_seq)
       << ",\"type\":\"response\""
       << ",\"request_seq\":" << requestSeq
       << ",\"success\":" << (success ? "true" : "false")
       << ",\"command\":\"" << command << "\""
       << ",\"body\":" << body << "}";
    writeMessage(ss.str());
}

void Debugger::sendEvent(const std::string& event, const std::string& body) {
    std::ostringstream ss;
    ss << "{\"seq\":" << (++m_seq)
       << ",\"type\":\"event\""
       << ",\"event\":\"" << event << "\""
       << ",\"body\":" << body << "}";
    writeMessage(ss.str());
}

void Debugger::processMessage(const std::string& json) {
    // Parse request
    std::string type = getJsonString(json, "type");
    std::string command = getJsonString(json, "command");
    int seq = getJsonInt(json, "seq");

    // Extract arguments
    size_t argsPos = json.find("\"arguments\":");
    std::string args = "{}";
    if (argsPos != std::string::npos) {
        size_t start = json.find('{', argsPos);
        if (start != std::string::npos) {
            int depth = 0;
            size_t end = start;
            for (size_t i = start; i < json.length(); ++i) {
                if (json[i] == '{') depth++;
                else if (json[i] == '}') {
                    depth--;
                    if (depth == 0) {
                        end = i;
                        break;
                    }
                }
            }
            args = json.substr(start, end - start + 1);
        }
    }

    // Dispatch to handler
    if (command == "initialize") handleInitialize(seq, args);
    else if (command == "launch") handleLaunch(seq, args);
    else if (command == "attach") handleAttach(seq, args);
    else if (command == "disconnect") handleDisconnect(seq, args);
    else if (command == "setBreakpoints") handleSetBreakpoints(seq, args);
    else if (command == "setExceptionBreakpoints") handleSetExceptionBreakpoints(seq, args);
    else if (command == "configurationDone") handleConfigurationDone(seq, args);
    else if (command == "threads") handleThreads(seq, args);
    else if (command == "stackTrace") handleStackTrace(seq, args);
    else if (command == "scopes") handleScopes(seq, args);
    else if (command == "variables") handleVariables(seq, args);
    else if (command == "continue") handleContinue(seq, args);
    else if (command == "next") handleNext(seq, args);
    else if (command == "stepIn") handleStepIn(seq, args);
    else if (command == "stepOut") handleStepOut(seq, args);
    else if (command == "pause") handlePause(seq, args);
    else if (command == "terminate") handleTerminate(seq, args);
    else if (command == "evaluate") handleEvaluate(seq, args);
    else if (command == "source") handleSource(seq, args);
    else if (command == "setVariable") handleSetVariable(seq, args);
    else if (command == "dataBreakpointInfo") handleDataBreakpointInfo(seq, args);
    else if (command == "setDataBreakpoints") handleSetDataBreakpoints(seq, args);
    else if (command == "completions") handleCompletions(seq, args);
    else {
        sendResponse(seq, command, false, "{\"message\":\"Unknown command\"}");
    }
}

std::string Debugger::buildCapabilities() {
    return "{"
        "\"supportsConfigurationDoneRequest\":true,"
        "\"supportsSetVariable\":true,"
        "\"supportsConditionalBreakpoints\":true,"
        "\"supportsHitConditionalBreakpoints\":true,"
        "\"supportsEvaluateForHovers\":true,"
        "\"supportsStepBack\":false,"
        "\"supportsGotoTargetsRequest\":false,"
        "\"supportsStepInTargetsRequest\":false,"
        "\"supportsCompletionsRequest\":true,"
        "\"supportsRestartFrame\":false,"
        "\"supportsExceptionOptions\":false,"
        "\"supportsValueFormattingOptions\":true,"
        "\"supportsExceptionInfoRequest\":false,"
        "\"supportTerminateDebuggee\":true,"
        "\"supportsDelayedStackTraceLoading\":true,"
        "\"supportsLoadedSourcesRequest\":true,"
        "\"supportsLogPoints\":true,"
        "\"supportsTerminateThreadsRequest\":false,"
        "\"supportsSetExpression\":true,"
        "\"supportsTerminateRequest\":true,"
        "\"supportsDataBreakpoints\":false,"
        "\"supportsReadMemoryRequest\":false,"
        "\"supportsDisassembleRequest\":false,"
        "\"supportsCancelRequest\":false,"
        "\"supportsBreakpointLocationsRequest\":true,"
        "\"supportsClipboardContext\":true,"
        "\"supportsSteppingGranularity\":true"
        "}";
}

void Debugger::handleInitialize(int seq, const std::string& args) {
    m_initialized = true;
    sendResponse(seq, "initialize", true, buildCapabilities());
    sendEvent("initialized");
}

void Debugger::handleLaunch(int seq, const std::string& args) {
    m_programPath = getJsonString(args, "program");
    m_workingDir = getJsonString(args, "cwd");

    bool success = launchProcess(m_programPath, m_programArgs);
    sendResponse(seq, "launch", success);

    if (success) {
        sendEvent("process", "{\"name\":\"" + m_programPath + "\",\"startMethod\":\"launch\"}");
    }
}

void Debugger::handleAttach(int seq, const std::string& args) {
    sendResponse(seq, "attach", false, "{\"message\":\"Attach not supported\"}");
}

void Debugger::handleDisconnect(int seq, const std::string& args) {
    terminateProcess();
    sendResponse(seq, "disconnect", true);
    m_running = false;
}

void Debugger::handleSetBreakpoints(int seq, const std::string& args) {
    std::string source = getJsonString(args, "source");

    // Clear existing breakpoints for this source
    m_breakpoints.erase(
        std::remove_if(m_breakpoints.begin(), m_breakpoints.end(),
            [&source](const Breakpoint& bp) { return bp.source == source; }),
        m_breakpoints.end());

    // Parse and add new breakpoints
    std::ostringstream bpsJson;
    bpsJson << "{\"breakpoints\":[";

    // Simple parsing of breakpoints array
    size_t bpStart = args.find("\"breakpoints\":");
    if (bpStart != std::string::npos) {
        size_t arrStart = args.find('[', bpStart);
        size_t arrEnd = args.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos) {
            std::string bpArray = args.substr(arrStart + 1, arrEnd - arrStart - 1);

            bool first = true;
            size_t pos = 0;
            while ((pos = bpArray.find("\"line\":", pos)) != std::string::npos) {
                int line = std::stoi(bpArray.substr(pos + 7));

                Breakpoint bp;
                bp.id = m_nextBreakpointId++;
                bp.source = source;
                bp.line = line;
                bp.verified = true;
                m_breakpoints.push_back(bp);

                if (!first) bpsJson << ",";
                bpsJson << "{\"id\":" << bp.id
                       << ",\"verified\":true"
                       << ",\"line\":" << bp.line << "}";
                first = false;
                pos += 7;
            }
        }
    }

    bpsJson << "]}";
    sendResponse(seq, "setBreakpoints", true, bpsJson.str());
}

void Debugger::handleSetExceptionBreakpoints(int seq, const std::string& args) {
    sendResponse(seq, "setExceptionBreakpoints", true);
}

void Debugger::handleConfigurationDone(int seq, const std::string& args) {
    sendResponse(seq, "configurationDone", true);

    // Notify that we stopped at entry
    sendEvent("stopped", "{\"reason\":\"entry\",\"threadId\":1,\"allThreadsStopped\":true}");
}

void Debugger::handleThreads(int seq, const std::string& args) {
    sendResponse(seq, "threads", true,
        "{\"threads\":[{\"id\":1,\"name\":\"main thread\"}]}");
}

void Debugger::handleStackTrace(int seq, const std::string& args) {
    std::ostringstream ss;
    ss << "{\"stackFrames\":[";

    if (!m_programPath.empty()) {
        ss << "{\"id\":1,\"name\":\"main\","
           << "\"source\":{\"name\":\"" << m_programPath << "\",\"path\":\"" << m_programPath << "\"},"
           << "\"line\":1,\"column\":1}";
    }

    ss << "],\"totalFrames\":1}";
    sendResponse(seq, "stackTrace", true, ss.str());
}

void Debugger::handleScopes(int seq, const std::string& args) {
    sendResponse(seq, "scopes", true,
        "{\"scopes\":["
        "{\"name\":\"Locals\",\"variablesReference\":1,\"expensive\":false},"
        "{\"name\":\"Globals\",\"variablesReference\":2,\"expensive\":false}"
        "]}");
}

void Debugger::handleVariables(int seq, const std::string& args) {
    int ref = getJsonInt(args, "variablesReference");

    std::vector<Variable> vars;

    if (ref == 1) {
        // Locals scope
        vars = getLocalVariables(m_currentFrameId);
    } else if (ref == 2) {
        // Globals scope
        vars = getGlobalVariables();
    } else if (ref == 3) {
        // Watches scope (if supported)
        vars = getWatchVariables();
    }

    std::ostringstream ss;
    ss << "{\"variables\":[";

    bool first = true;
    for (const auto& v : vars) {
        if (!first) ss << ",";
        ss << "{\"name\":\"" << v.name << "\""
           << ",\"value\":\"" << v.value << "\""
           << ",\"type\":\"" << v.type << "\""
           << ",\"variablesReference\":" << v.variablesReference << "}";
        first = false;
    }

    ss << "]}";
    sendResponse(seq, "variables", true, ss.str());
}

void Debugger::handleContinue(int seq, const std::string& args) {
    sendResponse(seq, "continue", true, "{\"allThreadsContinued\":true}");

    // Simulate running to completion
    sendEvent("continued", "{\"threadId\":1,\"allThreadsContinued\":true}");
    sendEvent("terminated");
    m_terminated = true;
}

void Debugger::handleNext(int seq, const std::string& args) {
    sendResponse(seq, "next", true);
    sendEvent("stopped", "{\"reason\":\"step\",\"threadId\":1,\"allThreadsStopped\":true}");
}

void Debugger::handleStepIn(int seq, const std::string& args) {
    sendResponse(seq, "stepIn", true);
    sendEvent("stopped", "{\"reason\":\"step\",\"threadId\":1,\"allThreadsStopped\":true}");
}

void Debugger::handleStepOut(int seq, const std::string& args) {
    sendResponse(seq, "stepOut", true);
    sendEvent("stopped", "{\"reason\":\"step\",\"threadId\":1,\"allThreadsStopped\":true}");
}

void Debugger::handlePause(int seq, const std::string& args) {
    sendResponse(seq, "pause", true);
    sendEvent("stopped", "{\"reason\":\"pause\",\"threadId\":1,\"allThreadsStopped\":true}");
}

void Debugger::handleTerminate(int seq, const std::string& args) {
    terminateProcess();
    sendResponse(seq, "terminate", true);
    sendEvent("terminated");
    m_terminated = true;
}

void Debugger::handleEvaluate(int seq, const std::string& args) {
    std::string expr = getJsonString(args, "expression");
    sendResponse(seq, "evaluate", true,
        "{\"result\":\"" + expr + "\",\"variablesReference\":0}");
}

void Debugger::handleSource(int seq, const std::string& args) {
    sendResponse(seq, "source", false, "{\"message\":\"Source not available\"}");
}

// JSON parsing helpers
std::string Debugger::getJsonString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    size_t start = pos + search.length();
    size_t end = json.find('"', start);
    if (end == std::string::npos) return "";

    return json.substr(start, end - start);
}

int Debugger::getJsonInt(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return 0;

    size_t start = pos + search.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;

    try {
        return std::stoi(json.substr(start));
    } catch (...) {
        return 0;
    }
}

bool Debugger::getJsonBool(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return false;

    size_t start = pos + search.length();
    while (start < json.length() && (json[start] == ' ' || json[start] == '\t')) start++;

    return json.substr(start, 4) == "true";
}

// Process management
bool Debugger::launchProcess(const std::string& program, const std::vector<std::string>& args) {
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    std::string cmdLine = program;
    for (const auto& arg : args) {
        cmdLine += " " + arg;
    }

    if (!CreateProcessA(
            nullptr,
            const_cast<char*>(cmdLine.c_str()),
            nullptr,
            nullptr,
            FALSE,
            DEBUG_PROCESS | CREATE_NEW_CONSOLE,
            nullptr,
            m_workingDir.empty() ? nullptr : m_workingDir.c_str(),
            &si,
            &pi)) {
        return false;
    }

    m_processHandle = pi.hProcess;
    m_threadHandle = pi.hThread;
    m_processId = pi.dwProcessId;
    return true;
#else
    m_processPid = fork();
    if (m_processPid == 0) {
        // Child process
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(program.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        if (!m_workingDir.empty()) {
            chdir(m_workingDir.c_str());
        }

        execv(program.c_str(), argv.data());
        _exit(1);
    }
    return m_processPid > 0;
#endif
}

void Debugger::terminateProcess() {
#ifdef _WIN32
    if (m_processHandle) {
        TerminateProcess(m_processHandle, 0);
        CloseHandle(m_processHandle);
        CloseHandle(m_threadHandle);
        m_processHandle = nullptr;
        m_threadHandle = nullptr;
    }
#else
    if (m_processPid > 0) {
        kill(m_processPid, SIGTERM);
        waitpid(m_processPid, nullptr, 0);
        m_processPid = -1;
    }
#endif
}

bool Debugger::isProcessRunning() {
#ifdef _WIN32
    if (!m_processHandle) return false;
    DWORD exitCode;
    if (GetExitCodeProcess(m_processHandle, &exitCode)) {
        return exitCode == STILL_ACTIVE;
    }
    return false;
#else
    if (m_processPid <= 0) return false;
    int status;
    pid_t result = waitpid(m_processPid, &status, WNOHANG);
    return result == 0;
#endif
}

// ============================================================================
// New DAP handlers
// ============================================================================

void Debugger::handleSetVariable(int seq, const std::string& args) {
    int varRef = getJsonInt(args, "variablesReference");
    std::string name = getJsonString(args, "name");
    std::string value = getJsonString(args, "value");

    // For now, just acknowledge the set (actual implementation would modify debuggee)
    sendResponse(seq, "setVariable", true,
        "{\"value\":\"" + value + "\",\"variablesReference\":0}");
}

void Debugger::handleDataBreakpointInfo(int seq, const std::string& args) {
    std::string name = getJsonString(args, "name");
    sendResponse(seq, "dataBreakpointInfo", true,
        "{\"dataId\":null,\"description\":\"Data breakpoints not supported\",\"accessTypes\":[]}");
}

void Debugger::handleSetDataBreakpoints(int seq, const std::string& args) {
    sendResponse(seq, "setDataBreakpoints", true, "{\"breakpoints\":[]}");
}

void Debugger::handleCompletions(int seq, const std::string& args) {
    std::string text = getJsonString(args, "text");

    // Simple completion based on known symbols
    std::ostringstream ss;
    ss << "{\"targets\":[";

    bool first = true;
    for (const auto& sym : m_symbols) {
        if (sym.name.find(text) == 0) {
            if (!first) ss << ",";
            ss << "{\"label\":\"" << sym.name << "\","
               << "\"type\":\"" << (sym.isGlobal ? "variable" : "local") << "\"}";
            first = false;
        }
    }

    ss << "]}";
    sendResponse(seq, "completions", true, ss.str());
}

// ============================================================================
// Source mapping
// ============================================================================

bool Debugger::loadSourceMap(const std::string& mapFile) {
    std::ifstream file(mapFile);
    if (!file.is_open()) return false;

    m_sourceMappings.clear();
    std::string line;

    // Format: manaLine,manaCol,cppLine,manaFile,cppFile,functionName
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        SourceMapping mapping;
        std::istringstream ss(line);
        std::string token;

        if (!std::getline(ss, token, ',')) continue;
        mapping.manaLine = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        mapping.manaColumn = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        mapping.cppLine = std::stoi(token);

        if (!std::getline(ss, mapping.manaFile, ',')) continue;
        if (!std::getline(ss, mapping.cppFile, ',')) continue;
        std::getline(ss, mapping.functionName, ',');

        m_sourceMappings.push_back(mapping);
        m_manaToCpp[mapping.manaFile] = mapping.cppFile;
    }

    logDebug("Loaded " + std::to_string(m_sourceMappings.size()) + " source mappings");
    return true;
}

bool Debugger::generateSourceMap(const std::string& manaFile, const std::string& cppFile) {
    // Parse both files and generate simple line-by-line mapping
    std::ifstream mana(manaFile);
    std::ifstream cpp(cppFile);

    if (!mana.is_open() || !cpp.is_open()) return false;

    int manaLine = 0;
    int cppLine = 0;
    std::string line;
    std::string currentFunc;

    // Read C++ file and look for // Line X comments
    while (std::getline(cpp, line)) {
        cppLine++;

        // Look for function declaration
        size_t funcPos = line.find("int32_t ");
        if (funcPos == std::string::npos) funcPos = line.find("void ");
        if (funcPos != std::string::npos) {
            size_t nameStart = line.find(' ', funcPos) + 1;
            size_t nameEnd = line.find('(', nameStart);
            if (nameEnd != std::string::npos) {
                currentFunc = line.substr(nameStart, nameEnd - nameStart);
            }
        }

        // Look for // Mana:line or // Line X markers
        size_t markerPos = line.find("// Line ");
        if (markerPos != std::string::npos) {
            int srcLine = std::stoi(line.substr(markerPos + 8));
            SourceMapping mapping;
            mapping.manaLine = srcLine;
            mapping.manaColumn = 1;
            mapping.cppLine = cppLine;
            mapping.manaFile = manaFile;
            mapping.cppFile = cppFile;
            mapping.functionName = currentFunc;
            m_sourceMappings.push_back(mapping);
        }
    }

    m_manaToCpp[manaFile] = cppFile;
    return true;
}

std::optional<SourceMapping> Debugger::manaLineToCppLine(const std::string& file, int line) {
    for (const auto& mapping : m_sourceMappings) {
        if (mapping.manaFile == file && mapping.manaLine == line) {
            return mapping;
        }
    }
    // Return approximate mapping (same line number)
    auto it = m_manaToCpp.find(file);
    if (it != m_manaToCpp.end()) {
        SourceMapping approx;
        approx.manaLine = line;
        approx.manaColumn = 1;
        approx.cppLine = line;
        approx.manaFile = file;
        approx.cppFile = it->second;
        return approx;
    }
    return std::nullopt;
}

std::optional<SourceMapping> Debugger::cppLineToManaLine(const std::string& file, int line) {
    for (const auto& mapping : m_sourceMappings) {
        if (mapping.cppFile == file && mapping.cppLine == line) {
            return mapping;
        }
    }
    // Find closest mapping
    int closest = -1;
    const SourceMapping* closestMapping = nullptr;
    for (const auto& mapping : m_sourceMappings) {
        if (mapping.cppFile == file && mapping.cppLine <= line) {
            if (closest == -1 || mapping.cppLine > closest) {
                closest = mapping.cppLine;
                closestMapping = &mapping;
            }
        }
    }
    if (closestMapping) return *closestMapping;
    return std::nullopt;
}

// ============================================================================
// Variable inspection
// ============================================================================

void Debugger::loadDebugSymbols(const std::string& symbolFile) {
    std::ifstream file(symbolFile);
    if (!file.is_open()) return;

    m_symbols.clear();
    m_symbolsByName.clear();
    std::string line;

    // Format: name,type,manaType,scope,line,isParam,isGlobal
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        DebugSymbol sym;
        std::istringstream ss(line);
        std::string token;

        if (!std::getline(ss, sym.name, ',')) continue;
        if (!std::getline(ss, sym.type, ',')) continue;
        if (!std::getline(ss, sym.manaType, ',')) continue;

        if (!std::getline(ss, token, ',')) continue;
        sym.scopeLevel = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        sym.declarationLine = std::stoi(token);

        if (!std::getline(ss, token, ',')) continue;
        sym.isParameter = (token == "1" || token == "true");

        if (!std::getline(ss, token, ',')) continue;
        sym.isGlobal = (token == "1" || token == "true");

        m_symbols.push_back(sym);
        m_symbolsByName[sym.name] = sym;
    }

    logDebug("Loaded " + std::to_string(m_symbols.size()) + " debug symbols");
}

std::vector<Variable> Debugger::getLocalVariables(int frameId) {
    std::vector<Variable> vars;

    // Return mock variables for demo
    // In production, would query the actual debuggee process
    Variable v1;
    v1.name = "x";
    v1.value = "42";
    v1.type = "i32";
    v1.variablesReference = 0;
    vars.push_back(v1);

    Variable v2;
    v2.name = "name";
    v2.value = "\"Mana\"";
    v2.type = "string";
    v2.variablesReference = 0;
    vars.push_back(v2);

    // Add known symbols at current scope
    for (const auto& sym : m_symbols) {
        if (!sym.isGlobal && !sym.isParameter && sym.declarationLine <= m_currentLine) {
            Variable v;
            v.name = sym.name;
            v.value = "<unknown>";
            v.type = sym.manaType;
            v.variablesReference = 0;
            vars.push_back(v);
        }
    }

    return vars;
}

std::vector<Variable> Debugger::getGlobalVariables() {
    std::vector<Variable> vars;

    for (const auto& sym : m_symbols) {
        if (sym.isGlobal) {
            Variable v;
            v.name = sym.name;
            v.value = "<global>";
            v.type = sym.manaType;
            v.variablesReference = 0;
            vars.push_back(v);
        }
    }

    return vars;
}

std::string Debugger::evaluateExpression(const std::string& expr, int frameId) {
    // Simple expression evaluation
    // In production, would send expression to debuggee for evaluation

    // Check if it's a known variable
    auto it = m_symbolsByName.find(expr);
    if (it != m_symbolsByName.end()) {
        return formatValue("<" + it->second.manaType + ">", it->second.type);
    }

    // Handle simple literals
    if (expr.find_first_not_of("0123456789.-") == std::string::npos) {
        return expr;  // It's a number
    }

    if (expr.front() == '"' && expr.back() == '"') {
        return expr;  // It's a string
    }

    // Check for simple operations
    if (expr.find('+') != std::string::npos ||
        expr.find('-') != std::string::npos ||
        expr.find('*') != std::string::npos ||
        expr.find('/') != std::string::npos) {
        return "<expression>";
    }

    return "<cannot evaluate: " + expr + ">";
}

std::string Debugger::formatValue(const std::string& value, const std::string& type) {
    // Format value based on type
    if (type == "string" || type == "std::string") {
        if (value.front() != '"') {
            return "\"" + value + "\"";
        }
    }
    if (type == "bool") {
        return (value == "1" || value == "true") ? "true" : "false";
    }
    return value;
}

// ============================================================================
// Watch management
// ============================================================================

int Debugger::addWatch(const std::string& expression) {
    Watch w;
    w.id = m_nextWatchId++;
    w.expression = expression;
    w.lastValue = "";
    w.hasChanged = false;
    m_watches.push_back(w);
    return w.id;
}

void Debugger::removeWatch(int id) {
    m_watches.erase(
        std::remove_if(m_watches.begin(), m_watches.end(),
            [id](const Watch& w) { return w.id == id; }),
        m_watches.end());
}

void Debugger::updateWatches(int frameId) {
    for (auto& watch : m_watches) {
        std::string newValue = evaluateExpression(watch.expression, frameId);
        watch.hasChanged = (newValue != watch.lastValue);
        watch.lastValue = newValue;
    }
}

std::vector<Variable> Debugger::getWatchVariables() {
    std::vector<Variable> vars;

    for (const auto& watch : m_watches) {
        Variable v;
        v.name = watch.expression;
        v.value = watch.lastValue.empty() ? "<not evaluated>" : watch.lastValue;
        v.type = "";
        v.variablesReference = 0;
        vars.push_back(v);
    }

    return vars;
}

// ============================================================================
// Breakpoint management
// ============================================================================

bool Debugger::checkBreakpointCondition(const Breakpoint& bp) {
    if (bp.condition.empty()) return true;

    // Evaluate condition
    std::string result = evaluateExpression(bp.condition, m_currentFrameId);
    return (result == "true" || result == "1");
}

void Debugger::updateBreakpointHitCount(int bpId) {
    for (auto& bp : m_breakpoints) {
        if (bp.id == bpId) {
            bp.hitCount++;

            // Check hit condition
            if (!bp.hitCondition.empty()) {
                int required = std::stoi(bp.hitCondition);
                if (bp.hitCount < required) {
                    // Don't break yet
                }
            }

            // Handle log point
            if (!bp.logMessage.empty()) {
                std::string msg = bp.logMessage;
                // Replace {} with evaluated expression
                // sendEvent("output", "{\"category\":\"console\",\"output\":\"" + msg + "\\n\"}");
            }
            break;
        }
    }
}

std::vector<int> Debugger::getBreakpointsAtLine(const std::string& file, int line) {
    std::vector<int> ids;
    for (const auto& bp : m_breakpoints) {
        if (bp.source == file && bp.line == line && bp.enabled) {
            ids.push_back(bp.id);
        }
    }
    return ids;
}

// ============================================================================
// JSON parsing helper
// ============================================================================

std::vector<std::string> Debugger::getJsonArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;

    std::string search = "\"" + key + "\":[";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return result;

    size_t start = pos + search.length();
    size_t end = json.find(']', start);
    if (end == std::string::npos) return result;

    std::string arrayContent = json.substr(start, end - start);

    // Simple parsing of string array
    size_t strStart = 0;
    while ((strStart = arrayContent.find('"', strStart)) != std::string::npos) {
        size_t strEnd = arrayContent.find('"', strStart + 1);
        if (strEnd != std::string::npos) {
            result.push_back(arrayContent.substr(strStart + 1, strEnd - strStart - 1));
            strStart = strEnd + 1;
        } else {
            break;
        }
    }

    return result;
}

// ============================================================================
// Debug logging
// ============================================================================

void Debugger::logDebug(const std::string& message) {
    if (m_enableLogging) {
        std::cerr << "[Mana Debug] " << message << std::endl;
    }
}

} // namespace debug
} // namespace mana
