// Mana Debug Adapter - Debug Adapter Protocol implementation
// Entry point for mana-debug executable

#include "Debugger.h"
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char* argv[]) {
    // Set stdin/stdout to binary mode on Windows
    // This is required for proper DAP message handling
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    // Check for version flag
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "-v" || arg == "--version") {
            std::cout << "mana-debug 1.0.0" << std::endl;
            return 0;
        }
        if (arg == "-h" || arg == "--help") {
            std::cout << "Mana Debug Adapter v1.0.0\n"
                      << "Debug Adapter Protocol (DAP) server for the Mana language.\n"
                      << "\n"
                      << "Usage: mana-debug [options]\n"
                      << "\n"
                      << "Options:\n"
                      << "  -h, --help     Show this help message\n"
                      << "  -v, --version  Show version\n"
                      << "\n"
                      << "The debug adapter communicates via stdin/stdout using the\n"
                      << "Debug Adapter Protocol. It is typically launched by an IDE\n"
                      << "or editor extension.\n"
                      << std::endl;
            return 0;
        }
    }

    // Create and run the debugger
    mana::debug::Debugger debugger;
    debugger.run();

    return 0;
}
