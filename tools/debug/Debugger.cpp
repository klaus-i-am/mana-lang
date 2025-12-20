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
    , m_seq(0)
    , m_nextBreakpointId(1)
    , m_nextVariableRef(1)
#ifdef _WIN32
    , m_processHandle(nullptr)
    , m_threadHandle(nullptr)
    , m_processId(0)
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
        "\"supportsCompletionsRequest\":false,"
        "\"supportsRestartFrame\":false,"
        "\"supportsExceptionOptions\":false,"
        "\"supportsValueFormattingOptions\":true,"
        "\"supportsExceptionInfoRequest\":false,"
        "\"supportTerminateDebuggee\":true,"
        "\"supportsDelayedStackTraceLoading\":true,"
        "\"supportsLoadedSourcesRequest\":false,"
        "\"supportsLogPoints\":true,"
        "\"supportsTerminateThreadsRequest\":false,"
        "\"supportsSetExpression\":false,"
        "\"supportsTerminateRequest\":true,"
        "\"supportsDataBreakpoints\":false,"
        "\"supportsReadMemoryRequest\":false,"
        "\"supportsDisassembleRequest\":false,"
        "\"supportsCancelRequest\":false,"
        "\"supportsBreakpointLocationsRequest\":false"
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

    // Return empty variables for now
    sendResponse(seq, "variables", true, "{\"variables\":[]}");
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

} // namespace debug
} // namespace mana
