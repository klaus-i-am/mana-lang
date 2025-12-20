#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <optional>

namespace mana {
namespace debug {

// Breakpoint information
struct Breakpoint {
    int id;
    std::string source;
    int line;
    bool verified;
    std::string condition;
    std::string hitCondition;
    std::string logMessage;
};

// Stack frame information
struct StackFrame {
    int id;
    std::string name;
    std::string source;
    int line;
    int column;
};

// Variable information
struct Variable {
    std::string name;
    std::string value;
    std::string type;
    int variablesReference;
};

// Scope information
struct Scope {
    std::string name;
    int variablesReference;
    bool expensive;
};

// Thread information
struct Thread {
    int id;
    std::string name;
};

// Debug event types
enum class StopReason {
    Step,
    Breakpoint,
    Exception,
    Pause,
    Entry,
    Exit
};

// DAP message types
struct DAPMessage {
    int seq;
    std::string type;
    std::string command;
    std::map<std::string, std::string> arguments;
    bool success;
    std::string message;
    std::string body;
};

// Debugger class - implements Debug Adapter Protocol
class Debugger {
public:
    Debugger();
    ~Debugger();

    // Main run loop
    void run();

    // Process a single DAP message
    void processMessage(const std::string& json);

    // Send response
    void sendResponse(int requestSeq, const std::string& command,
                     bool success, const std::string& body = "{}");

    // Send event
    void sendEvent(const std::string& event, const std::string& body = "{}");

    // DAP request handlers
    void handleInitialize(int seq, const std::string& args);
    void handleLaunch(int seq, const std::string& args);
    void handleAttach(int seq, const std::string& args);
    void handleDisconnect(int seq, const std::string& args);
    void handleSetBreakpoints(int seq, const std::string& args);
    void handleSetExceptionBreakpoints(int seq, const std::string& args);
    void handleConfigurationDone(int seq, const std::string& args);
    void handleThreads(int seq, const std::string& args);
    void handleStackTrace(int seq, const std::string& args);
    void handleScopes(int seq, const std::string& args);
    void handleVariables(int seq, const std::string& args);
    void handleContinue(int seq, const std::string& args);
    void handleNext(int seq, const std::string& args);
    void handleStepIn(int seq, const std::string& args);
    void handleStepOut(int seq, const std::string& args);
    void handlePause(int seq, const std::string& args);
    void handleTerminate(int seq, const std::string& args);
    void handleEvaluate(int seq, const std::string& args);
    void handleSource(int seq, const std::string& args);

private:
    // Read a DAP message from stdin
    std::optional<std::string> readMessage();

    // Write a DAP message to stdout
    void writeMessage(const std::string& json);

    // Parse JSON (simple implementation)
    std::string getJsonString(const std::string& json, const std::string& key);
    int getJsonInt(const std::string& json, const std::string& key);
    bool getJsonBool(const std::string& json, const std::string& key);

    // Build JSON response
    std::string buildCapabilities();

    // Process management
    bool launchProcess(const std::string& program, const std::vector<std::string>& args);
    void terminateProcess();
    bool isProcessRunning();

    // Debugging state
    bool m_initialized;
    bool m_running;
    bool m_terminated;
    int m_seq;

    // Breakpoints
    std::vector<Breakpoint> m_breakpoints;
    int m_nextBreakpointId;

    // Stack and variables
    std::vector<StackFrame> m_stackFrames;
    std::map<int, std::vector<Variable>> m_variables;
    int m_nextVariableRef;

    // Process info
    std::string m_programPath;
    std::vector<std::string> m_programArgs;
    std::string m_workingDir;

#ifdef _WIN32
    void* m_processHandle;
    void* m_threadHandle;
    unsigned long m_processId;
#else
    int m_processPid;
#endif
};

} // namespace debug
} // namespace mana
