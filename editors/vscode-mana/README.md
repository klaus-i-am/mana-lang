# Mana for Visual Studio Code

Official VS Code extension for the [Mana programming language](https://mana-lang.org).

## Features

- **Syntax Highlighting** - Full syntax highlighting for `.mana` files
- **Language Server** - Code completion, diagnostics, go-to-definition, and more
- **Debugging** - Integrated debugging with breakpoints and variable inspection
- **Commands** - Build, run, and test your Mana projects

## Requirements

- [Mana compiler](https://mana-lang.org/install) installed and in PATH
- `mana-lsp` for language server features (included with Mana)
- `mana-debug` for debugging (included with Mana)

## Installation

1. Install from the VS Code Marketplace
2. Or download the `.vsix` file and install manually:
   ```
   code --install-extension mana-lang-1.0.2.vsix
   ```

## Configuration

| Setting | Default | Description |
|---------|---------|-------------|
| `mana.path` | `mana` | Path to Mana compiler |
| `mana.lspPath` | `mana-lsp` | Path to language server |
| `mana.enableLsp` | `true` | Enable language server |

## Commands

- **Mana: Build Project** - Build the current project
- **Mana: Run Project** - Build and run the project
- **Mana: Run Tests** - Run all tests
- **Mana: Restart Language Server** - Restart the LSP

## Debugging

Create a `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "mana",
      "request": "launch",
      "name": "Debug Mana",
      "program": "${workspaceFolder}/main.mana"
    }
  ]
}
```

## Links

- [Mana Website](https://mana-lang.org)
- [Documentation](https://mana-lang.org/docs)
- [GitHub](https://github.com/klaus-i-am/mana-lang)
- [Report Issues](https://github.com/klaus-i-am/mana-lang/issues)

## License

MIT
