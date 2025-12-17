# Mana Tools

# LSP Server
add_executable(mana-lsp
    lsp/main.cpp
    lsp/LspServer.cpp
)
target_include_directories(mana-lsp PRIVATE ${CMAKE_SOURCE_DIR}/frontend)
target_link_libraries(mana-lsp mana_frontend)

# Formatter library (linked into main compiler)
add_library(mana_fmt STATIC
    fmt/Formatter.cpp
)
target_include_directories(mana_fmt PRIVATE ${CMAKE_SOURCE_DIR}/frontend)

# REPL library (linked into main compiler)
add_library(mana_repl STATIC
    repl/Repl.cpp
)
target_include_directories(mana_repl PRIVATE ${CMAKE_SOURCE_DIR}/frontend)

# Package manager library (linked into main compiler)
add_library(mana_pkg STATIC
    pkg/PackageManager.cpp
)
target_include_directories(mana_pkg PRIVATE ${CMAKE_SOURCE_DIR}/frontend)
