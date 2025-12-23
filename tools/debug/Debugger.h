#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <set>
#include <fstream>

namespace mana {
namespace debug {

// Source location mapping (Mana line -> C++ line)
struct SourceMapping {
    int manaLine;
    int manaColumn;
    int cppLine;
    std::string manaFile;
    std::string cppFile;
    std::string functionName;
};

// Debug symbol for a variable
struct DebugSymbol {
    std::string name;
    std::string type;
    std::string manaType;  // Original Mana type
    int scopeLevel;
    int declarationLine;
    bool isParameter;
    bool isGlobal;
};

// Watch expression
struct Watch {
    int id;
    std::string expression;
    std::string lastValue;
    bool hasChanged;
};

// Breakpoint information
struct Breakpoint {
    int id;
    std::string source;
    int line;
    bool verified;
    std::string condition;
    std::string hitCondition;
    std::string logMessage;
    int hitCount;
    bool enabled;
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
    void handleSetVariable(int seq, const std::string& args);
    void handleDataBreakpointInfo(int seq, const std::string& args);
    void handleSetDataBreakpoints(int seq, const std::string& args);
    void handleCompletions(int seq, const std::string& args);

private:
    // Read a DAP message from stdin
    std::optional<std::string> readMessage();

    // Write a DAP message to stdout
    void writeMessage(const std::string& json);

    // Parse JSON (simple implementation)
    std::string getJsonString(const std::string& json, const std::string& key);
    int getJsonInt(const std::string& json, const std::string& key);
    bool getJsonBool(const std::string& json, const std::string& key);
    std::vector<std::string> getJsonArray(const std::string& json, const std::string& key);

    // Build JSON response
    std::string buildCapabilities();

    // Process management
    bool launchProcess(const std::string& program, const std::vector<std::string>& args);
    void terminateProcess();
    bool isProcessRunning();

    // Source mapping
    bool loadSourceMap(const std::string& mapFile);
    bool generateSourceMap(const std::string& manaFile, const std::string& cppFile);
    std::optional<SourceMapping> manaLineToCppLine(const std::string& file, int line);
    std::optional<SourceMapping> cppLineToManaLine(const std::string& file, int line);

    // Variable inspection
    void loadDebugSymbols(const std::string& symbolFile);
    std::vector<Variable> getLocalVariables(int frameId);
    std::vector<Variable> getGlobalVariables();
    std::string evaluateExpression(const std::string& expr, int frameId);
    std::string formatValue(const std::string& value, const std::string& type);

    // Watch management
    int addWatch(const std::string& expression);
    void removeWatch(int id);
    void updateWatches(int frameId);
    std::vector<Variable> getWatchVariables();

    // Breakpoint management
    bool checkBreakpointCondition(const Breakpoint& bp);
    void updateBreakpointHitCount(int bpId);
    std::vector<int> getBreakpointsAtLine(const std::string& file, int line);

    // Debug logging
    void logDebug(const std::string& message);

    // Debugging state
    bool m_initialized;
    bool m_running;
    bool m_terminated;
    bool m_paused;
    int m_seq;
    int m_currentLine;
    int m_currentFrameId;
    std::string m_currentFunction;

    // Breakpoints
    std::vector<Breakpoint> m_breakpoints;
    int m_nextBreakpointId;

    // Watches
    std::vector<Watch> m_watches;
    int m_nextWatchId;

    // Stack and variables
    std::vector<StackFrame> m_stackFrames;
    std::map<int, std::vector<Variable>> m_variables;
    int m_nextVariableRef;

    // Source mappings
    std::vector<SourceMapping> m_sourceMappings;
    std::unordered_map<std::string, std::string> m_manaToCpp;  // Mana file -> C++ file

    // Debug symbols
    std::vector<DebugSymbol> m_symbols;
    std::unordered_map<std::string, DebugSymbol> m_symbolsByName;

    // Process info
    std::string m_programPath;
    std::string m_manaSourcePath;  // Original .mana file
    std::vector<std::string> m_programArgs;
    std::string m_workingDir;
    bool m_stopOnEntry;
    bool m_enableLogging;

#ifdef _WIN32
    void* m_processHandle;
    void* m_threadHandle;
    unsigned long m_processId;
    void* m_debugEvent;
#else
    int m_processPid;
    int m_pipeToChild[2];
    int m_pipeFromChild[2];
#endif
};

} // namespace debug
} // namespace mana
