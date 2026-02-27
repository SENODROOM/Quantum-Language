"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const vscode = require("vscode");
function activate(context) {
    console.log('Quantum Language extension is now active!');
    // Register completion provider
    const provider = vscode.languages.registerCompletionItemProvider({ scheme: 'file', language: 'quantum' }, new QuantumCompletionProvider(), '.');
    context.subscriptions.push(provider);
}
exports.activate = activate;
class QuantumCompletionProvider {
    provideCompletionItems(document, position, token, context) {
        const linePrefix = document.lineAt(position).text.substring(0, position.character);
        // Keywords
        const keywords = [
            'let', 'const', 'fn', 'return', 'if', 'else', 'elif',
            'while', 'for', 'in', 'break', 'continue', 'print',
            'input', 'import', 'and', 'or', 'not', 'true', 'false', 'nil'
        ];
        // Cybersecurity keywords
        const cyberKeywords = ['scan', 'payload', 'encrypt', 'decrypt', 'hash'];
        // Built-in functions
        const builtInFunctions = [
            'num()', 'str()', 'bool()', 'abs()', 'sqrt()', 'floor()',
            'ceil()', 'round()', 'pow()', 'log()', 'log2()', 'sin()',
            'cos()', 'tan()', 'min()', 'max()'
        ];
        const completions = [];
        // Add keywords
        keywords.forEach(keyword => {
            const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            item.documentation = new vscode.MarkdownString(`Quantum keyword: \`${keyword}\``);
            completions.push(item);
        });
        // Add cybersecurity keywords
        cyberKeywords.forEach(keyword => {
            const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            item.documentation = new vscode.MarkdownString(`Cybersecurity keyword: \`${keyword}\``);
            item.detail = 'Reserved for future cybersecurity features';
            completions.push(item);
        });
        // Add built-in functions
        builtInFunctions.forEach(func => {
            const item = new vscode.CompletionItem(func, vscode.CompletionItemKind.Function);
            item.documentation = new vscode.MarkdownString(`Built-in function: \`${func}\``);
            completions.push(item);
        });
        // Add contextual completions
        if (linePrefix.endsWith('let ')) {
            const item = new vscode.CompletionItem('variable_name', vscode.CompletionItemKind.Variable);
            item.insertText = new vscode.SnippetString('${1:name} = ${2:value}');
            completions.push(item);
        }
        if (linePrefix.endsWith('fn ')) {
            const item = new vscode.CompletionItem('function_name', vscode.CompletionItemKind.Function);
            item.insertText = new vscode.SnippetString('${1:name}(${2:args}) {\n    ${3:// body}\n}');
            completions.push(item);
        }
        return completions;
    }
}
function deactivate() { }
exports.deactivate = deactivate;
//# sourceMappingURL=extension.js.map