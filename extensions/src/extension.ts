import * as vscode from 'vscode';

// ─── Activation ──────────────────────────────────────────────────────────────

export function activate(context: vscode.ExtensionContext) {
    console.log('Quantum Language extension v1.1.0 is now active!');

    context.subscriptions.push(
        vscode.languages.registerCompletionItemProvider(
            { scheme: 'file', language: 'quantum' },
            new QuantumCompletionProvider(),
            '.', '"', "'"
        ),
        vscode.languages.registerHoverProvider(
            { scheme: 'file', language: 'quantum' },
            new QuantumHoverProvider()
        ),
        vscode.languages.registerSignatureHelpProvider(
            { scheme: 'file', language: 'quantum' },
            new QuantumSignatureHelpProvider(),
            '(', ','
        )
    );
}

export function deactivate() {}

// ─── Completion Data ──────────────────────────────────────────────────────────

const KEYWORDS: Record<string, string> = {
    'let':      'Declare a mutable variable: `let x = 42`',
    'const':    'Declare an immutable constant: `const MAX = 100`',
    'fn':       'Define a Quantum function: `fn name(args) { ... }`',
    'def':      'Define a function (Python style): `def name(args):`',
    'function': 'Define a function (JS style): `function name(args) { ... }`',
    'return':   'Return a value from a function',
    'if':       'Conditional branch: `if condition { ... }`',
    'elif':     'Else-if branch (Python style): `elif condition:`',
    'else':     'Else branch',
    'for':      'For-in loop: `for item in collection { ... }`',
    'while':    'While loop: `while condition { ... }`',
    'break':    'Exit the current loop',
    'continue': 'Skip to next iteration',
    'in':       'Membership / iteration operator: `for x in arr`',
    'and':      'Logical AND operator',
    'or':       'Logical OR operator',
    'not':      'Logical NOT operator',
    'import':   'Import a module: `import "module.sa"`',
    'class':    'Define a class: `class Name { fn init(...) { ... } }`',
    'extends':  'Inherit from a parent class: `class Dog extends Animal`',
    'self':     'Reference to the current class instance',
    'true':     'Boolean true literal',
    'false':    'Boolean false literal',
    'null':     'Null / absence of value',
    'nil':      'Null / absence of value (alias)',
    'int':      'C-style integer type annotation',
    'float':    'C-style float type annotation',
    'double':   'C-style double type annotation',
    'string':   'C-style string type annotation',
    'bool':     'C-style boolean type annotation',
    'char':     'C-style char type annotation',
};

const CYBER_KEYWORDS: Record<string, string> = {
    'scan':    '🔐 **Reserved** — Network scanning (coming in v2.0)',
    'payload': '🔐 **Reserved** — Exploit payload construction (coming in v2.0)',
    'encrypt': '🔐 **Reserved** — Symmetric encryption (coming in v2.0)',
    'decrypt': '🔐 **Reserved** — Symmetric decryption (coming in v2.0)',
    'hash':    '🔐 **Reserved** — Hashing (MD5, SHA-256, SHA-512) (coming in v2.0)',
};

interface FunctionDoc {
    signature: string;
    doc: string;
    params?: string[];
}

const BUILTINS: Record<string, FunctionDoc> = {
    // I/O
    'print':         { signature: 'print(...args)',              doc: 'Print values to stdout, space-separated',          params: ['...args'] },
    'printf':        { signature: 'printf(fmt, ...args)',        doc: 'C-style formatted print',                          params: ['fmt: string', '...args'] },
    'scanf':         { signature: 'scanf(fmt, &var)',            doc: 'C-style formatted input',                          params: ['fmt: string', '&var'] },
    'input':         { signature: 'input(prompt)',               doc: 'Read a line of user input (returns string)',        params: ['prompt: string'] },
    'format':        { signature: 'format(fmt, ...args)',        doc: 'sprintf-style string formatting',                  params: ['fmt: string', '...args'] },

    // Type conversion
    'num':           { signature: 'num(x)',                      doc: 'Convert value to number',                          params: ['x'] },
    'str':           { signature: 'str(x)',                      doc: 'Convert value to string',                          params: ['x'] },
    'bool':          { signature: 'bool(x)',                     doc: 'Convert value to boolean',                         params: ['x'] },
    'type':          { signature: 'type(x)',                     doc: 'Returns the type name of x as a string',           params: ['x'] },
    'chr':           { signature: 'chr(n)',                      doc: 'Convert integer to character',                     params: ['n: int'] },
    'ord':           { signature: 'ord(c)',                      doc: 'Convert character to its integer code',            params: ['c: char'] },

    // Math
    'abs':           { signature: 'abs(x)',                      doc: 'Absolute value of x',                              params: ['x: number'] },
    'sqrt':          { signature: 'sqrt(x)',                     doc: 'Square root of x',                                 params: ['x: number'] },
    'pow':           { signature: 'pow(x, y)',                   doc: 'Raise x to the power y',                           params: ['x: number', 'y: number'] },
    'floor':         { signature: 'floor(x)',                    doc: 'Round x down to nearest integer',                  params: ['x: number'] },
    'ceil':          { signature: 'ceil(x)',                     doc: 'Round x up to nearest integer',                    params: ['x: number'] },
    'round':         { signature: 'round(x)',                    doc: 'Round x to nearest integer',                       params: ['x: number'] },
    'log':           { signature: 'log(x)',                      doc: 'Natural logarithm of x',                           params: ['x: number'] },
    'log2':          { signature: 'log2(x)',                     doc: 'Base-2 logarithm of x',                            params: ['x: number'] },
    'sin':           { signature: 'sin(x)',                      doc: 'Sine of x (radians)',                              params: ['x: number'] },
    'cos':           { signature: 'cos(x)',                      doc: 'Cosine of x (radians)',                            params: ['x: number'] },
    'tan':           { signature: 'tan(x)',                      doc: 'Tangent of x (radians)',                           params: ['x: number'] },
    'min':           { signature: 'min(...args)',                 doc: 'Minimum of given values',                          params: ['...args'] },
    'max':           { signature: 'max(...args)',                 doc: 'Maximum of given values',                          params: ['...args'] },

    // Utility
    'len':           { signature: 'len(x)',                      doc: 'Length of string, array, or dict',                 params: ['x'] },
    'range':         { signature: 'range(n) or range(a, b)',     doc: 'Generate integer range. `range(5)` → 0..4',        params: ['start?', 'end'] },
    'rand':          { signature: 'rand() or rand(a, b)',        doc: 'Random float [0,1] or in range [a,b]',             params: ['a?: number', 'b?: number'] },
    'rand_int':      { signature: 'rand_int(a, b)',              doc: 'Random integer in inclusive range [a, b]',         params: ['a: int', 'b: int'] },
    'time':          { signature: 'time()',                      doc: 'Unix timestamp (seconds since epoch)',             params: [] },
    'sleep':         { signature: 'sleep(s)',                    doc: 'Pause execution for s seconds',                   params: ['s: number'] },
    'assert':        { signature: 'assert(cond, msg)',           doc: 'Throw error with msg if condition is false',       params: ['cond: bool', 'msg: string'] },
    'exit':          { signature: 'exit(code)',                  doc: 'Exit program with status code',                   params: ['code: int'] },
    'keys':          { signature: 'keys(dict)',                  doc: 'Return array of dictionary keys',                 params: ['dict'] },
    'values':        { signature: 'values(dict)',                doc: 'Return array of dictionary values',               params: ['dict'] },

    // Encoding / Cybersecurity
    'hex':           { signature: 'hex(n)',                      doc: 'Convert integer to hex string (e.g. `0xff`)',      params: ['n: int'] },
    'bin':           { signature: 'bin(n)',                      doc: 'Convert integer to binary string',                params: ['n: int'] },
    'to_hex':        { signature: 'to_hex(s)',                   doc: 'Convert string bytes to hex representation',      params: ['s: string'] },
    'from_hex':      { signature: 'from_hex(s)',                 doc: 'Convert hex string back to byte string',          params: ['s: string'] },
    'xor_bytes':     { signature: 'xor_bytes(a, b)',             doc: 'XOR two byte strings together',                  params: ['a: string', 'b: string'] },
    'base64_encode': { signature: 'base64_encode(s)',            doc: 'Base64-encode a string',                          params: ['s: string'] },
    'rot13':         { signature: 'rot13(s)',                    doc: 'Apply ROT13 cipher to string',                    params: ['s: string'] },
};

// ─── Completion Provider ──────────────────────────────────────────────────────

class QuantumCompletionProvider implements vscode.CompletionItemProvider {
    provideCompletionItems(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token: vscode.CancellationToken,
        _context: vscode.CompletionContext
    ): vscode.CompletionItem[] {
        const linePrefix = document.lineAt(position).text.substring(0, position.character);
        const completions: vscode.CompletionItem[] = [];

        // ── Contextual: after 'let ' ──────────────────────────────────────────
        if (/\blet\s+$/.test(linePrefix)) {
            const item = new vscode.CompletionItem('name = value', vscode.CompletionItemKind.Snippet);
            item.insertText = new vscode.SnippetString('${1:name} = ${2:value}');
            item.detail = 'Variable declaration';
            completions.push(item);
            return completions;
        }

        // ── Contextual: after 'fn ' ───────────────────────────────────────────
        if (/\bfn\s+$/.test(linePrefix)) {
            const item = new vscode.CompletionItem('name(args) { }', vscode.CompletionItemKind.Snippet);
            item.insertText = new vscode.SnippetString('${1:name}(${2:args}) {\n    ${3:// body}\n    return ${4:value}\n}');
            item.detail = 'Function definition';
            completions.push(item);
            return completions;
        }

        // ── Contextual: after 'def ' ──────────────────────────────────────────
        if (/\bdef\s+$/.test(linePrefix)) {
            const item = new vscode.CompletionItem('name(args):', vscode.CompletionItemKind.Snippet);
            item.insertText = new vscode.SnippetString('${1:name}(${2:args}):\n    ${3:pass}');
            item.detail = 'Python-style function definition';
            completions.push(item);
            return completions;
        }

        // ── Contextual: after 'class ' ────────────────────────────────────────
        if (/\bclass\s+$/.test(linePrefix)) {
            const basic = new vscode.CompletionItem('ClassName { }', vscode.CompletionItemKind.Snippet);
            basic.insertText = new vscode.SnippetString('${1:ClassName} {\n    fn init(${2:args}) {\n        self.${3:field} = ${4:value}\n    }\n}');
            basic.detail = 'Class definition';

            const ext = new vscode.CompletionItem('ClassName extends Parent { }', vscode.CompletionItemKind.Snippet);
            ext.insertText = new vscode.SnippetString('${1:Child} extends ${2:Parent} {\n    fn init(${3:args}) {\n        self.${4:field} = ${5:value}\n    }\n}');
            ext.detail = 'Class with inheritance';

            completions.push(basic, ext);
            return completions;
        }

        // ── Method completions after '.' ──────────────────────────────────────
        if (linePrefix.endsWith('.')) {
            const arrayMethods = ['push', 'pop', 'slice', 'map', 'filter', 'reduce',
                                  'sort', 'reverse', 'includes', 'index_of', 'join'];
            const stringMethods = ['trim', 'upper', 'lower', 'split', 'replace',
                                   'contains', 'starts_with', 'ends_with'];
            const dictMethods  = ['get', 'keys', 'values', 'remove'];

            [...arrayMethods, ...stringMethods, ...dictMethods].forEach(m => {
                const item = new vscode.CompletionItem(m, vscode.CompletionItemKind.Method);
                item.insertText = new vscode.SnippetString(`${m}(\${1})`);
                completions.push(item);
            });
            return completions;
        }

        // ── Keywords ──────────────────────────────────────────────────────────
        for (const [kw, doc] of Object.entries(KEYWORDS)) {
            const item = new vscode.CompletionItem(kw, vscode.CompletionItemKind.Keyword);
            item.documentation = new vscode.MarkdownString(doc);
            completions.push(item);
        }

        // ── Cybersecurity reserved keywords ───────────────────────────────────
        for (const [kw, doc] of Object.entries(CYBER_KEYWORDS)) {
            const item = new vscode.CompletionItem(kw, vscode.CompletionItemKind.Keyword);
            item.documentation = new vscode.MarkdownString(doc);
            item.detail = '🔐 Reserved — cybersecurity (v2.0)';
            completions.push(item);
        }

        // ── Built-in functions ────────────────────────────────────────────────
        for (const [name, info] of Object.entries(BUILTINS)) {
            const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
            item.detail = info.signature;
            item.documentation = new vscode.MarkdownString(`**${info.signature}**\n\n${info.doc}`);
            item.insertText = new vscode.SnippetString(
                info.params && info.params.length > 0
                    ? `${name}(\${1})`
                    : `${name}()`
            );
            completions.push(item);
        }

        // ── Constants ─────────────────────────────────────────────────────────
        ['true', 'false', 'null', 'nil', 'PI', 'E', 'INF'].forEach(c => {
            const item = new vscode.CompletionItem(c, vscode.CompletionItemKind.Constant);
            completions.push(item);
        });

        return completions;
    }
}

// ─── Hover Provider ───────────────────────────────────────────────────────────

class QuantumHoverProvider implements vscode.HoverProvider {
    provideHover(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token: vscode.CancellationToken
    ): vscode.Hover | undefined {
        const range = document.getWordRangeAtPosition(position);
        if (!range) { return undefined; }
        const word = document.getText(range);

        if (BUILTINS[word]) {
            const info = BUILTINS[word];
            const md = new vscode.MarkdownString();
            md.appendCodeblock(info.signature, 'quantum');
            md.appendMarkdown(`\n${info.doc}`);
            return new vscode.Hover(md, range);
        }
        if (KEYWORDS[word]) {
            const md = new vscode.MarkdownString(`**Keyword:** \`${word}\`\n\n${KEYWORDS[word]}`);
            return new vscode.Hover(md, range);
        }
        if (CYBER_KEYWORDS[word]) {
            const md = new vscode.MarkdownString(`**Reserved Keyword:** \`${word}\`\n\n${CYBER_KEYWORDS[word]}`);
            return new vscode.Hover(md, range);
        }

        return undefined;
    }
}

// ─── Signature Help Provider ─────────────────────────────────────────────────

class QuantumSignatureHelpProvider implements vscode.SignatureHelpProvider {
    provideSignatureHelp(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token: vscode.CancellationToken,
        _context: vscode.SignatureHelpContext
    ): vscode.SignatureHelp | undefined {
        const lineText  = document.lineAt(position).text.substring(0, position.character);
        const callMatch = lineText.match(/(\w+)\s*\([^)]*$/);
        if (!callMatch) { return undefined; }

        const fnName = callMatch[1];
        const info   = BUILTINS[fnName];
        if (!info) { return undefined; }

        const sig   = new vscode.SignatureInformation(info.signature, new vscode.MarkdownString(info.doc));
        sig.parameters = (info.params ?? []).map(p => new vscode.ParameterInformation(p));

        const commas     = (lineText.match(/,/g) || []).length;
        const activeParam = Math.min(commas, sig.parameters.length - 1);

        const help = new vscode.SignatureHelp();
        help.signatures    = [sig];
        help.activeSignature = 0;
        help.activeParameter = activeParam;
        return help;
    }
}
