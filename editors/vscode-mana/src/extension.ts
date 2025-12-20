import * as vscode from 'vscode';
import * as path from 'path';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;

export function activate(context: vscode.ExtensionContext) {
    console.log('Mana extension activated');

    // Start LSP if enabled
    const config = vscode.workspace.getConfiguration('mana');
    if (config.get<boolean>('enableLsp', true)) {
        startLanguageServer(context);
    }

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('mana.build', runBuild),
        vscode.commands.registerCommand('mana.run', runProject),
        vscode.commands.registerCommand('mana.test', runTests),
        vscode.commands.registerCommand('mana.restartLsp', () => restartLanguageServer(context))
    );
}

function startLanguageServer(context: vscode.ExtensionContext) {
    const config = vscode.workspace.getConfiguration('mana');
    const lspPath = config.get<string>('lspPath', 'mana-lsp');

    const serverOptions: ServerOptions = {
        command: lspPath,
        args: [],
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'mana' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.mana')
        }
    };

    client = new LanguageClient(
        'mana-lsp',
        'Mana Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
    context.subscriptions.push({
        dispose: () => client?.stop()
    });
}

async function restartLanguageServer(context: vscode.ExtensionContext) {
    if (client) {
        await client.stop();
        client = undefined;
    }
    startLanguageServer(context);
    vscode.window.showInformationMessage('Mana Language Server restarted');
}

async function runBuild() {
    const config = vscode.workspace.getConfiguration('mana');
    const manaPath = config.get<string>('path', 'mana');

    const terminal = vscode.window.createTerminal('Mana Build');
    terminal.sendText(`${manaPath} build`);
    terminal.show();
}

async function runProject() {
    const config = vscode.workspace.getConfiguration('mana');
    const manaPath = config.get<string>('path', 'mana');

    const terminal = vscode.window.createTerminal('Mana Run');
    terminal.sendText(`${manaPath} run`);
    terminal.show();
}

async function runTests() {
    const config = vscode.workspace.getConfiguration('mana');
    const manaPath = config.get<string>('path', 'mana');

    const terminal = vscode.window.createTerminal('Mana Tests');
    terminal.sendText(`${manaPath} test`);
    terminal.show();
}

export function deactivate(): Thenable<void> | undefined {
    if (client) {
        return client.stop();
    }
    return undefined;
}
