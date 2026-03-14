import {
	createConnection,
	TextDocuments,
	Diagnostic,
	DiagnosticSeverity,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	CompletionItem,
	CompletionItemKind,
	TextDocumentPositionParams,
	TextDocumentSyncKind,
	InitializeResult
} from 'vscode-languageserver/node';

import {
	TextDocument
} from 'vscode-languageserver-textdocument';

import { exec } from 'child_process';
import * as path from 'path';
import * as fs from 'fs';

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);

// Create a simple text document manager.
const documents: TextDocuments<TextDocument> = new TextDocuments(TextDocument);

let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;

connection.onInitialize((params: InitializeParams) => {
	const capabilities = params.capabilities;

	// Does the client support the `workspace/configuration` request?
	// If not, we fall back using global settings.
	hasConfigurationCapability = !!(
		capabilities.workspace && !!capabilities.workspace.configuration
	);
	hasWorkspaceFolderCapability = !!(
		capabilities.workspace && !!capabilities.workspace.workspaceFolders
	);
	hasDiagnosticRelatedInformationCapability = !!(
		capabilities.textDocument &&
		capabilities.textDocument.publishDiagnostics &&
		capabilities.textDocument.publishDiagnostics.relatedInformation
	);

	const result: InitializeResult = {
		capabilities: {
			textDocumentSync: TextDocumentSyncKind.Incremental,
			// Tell the client that this server supports code completion.
			completionProvider: {
				resolveProvider: true
			},
			hoverProvider: true
		}
	};
	if (hasWorkspaceFolderCapability) {
		result.capabilities.workspace = {
			workspaceFolders: {
				supported: true
			}
		};
	}
	return result;
});

connection.onInitialized(() => {
	if (hasConfigurationCapability) {
		// Register for all configuration changes.
		connection.client.register(DidChangeConfigurationNotification.type, undefined);
	}
	if (hasWorkspaceFolderCapability) {
		connection.workspace.onDidChangeWorkspaceFolders(_event => {
			connection.console.log('Workspace folder change event received.');
		});
	}
});

// The content of a text document has changed. This event is emitted
// when the text document first opened or when its content has changed.
documents.onDidChangeContent(change => {
	validateTextDocument(change.document);
});

async function validateTextDocument(textDocument: TextDocument): Promise<void> {
	// In this implementation, we run the quantum compiler with --check flag
	const text = textDocument.getText();
	const filePath = new URL(textDocument.uri).pathname;
	// Convert URI to local path if needed (OS dependent)
	let localPath = filePath;
	if (process.platform === 'win32') {
		localPath = localPath.replace(/^\/([a-zA-Z]:)/, '$1').replace(/\//g, '\\');
	}

	// For a real LSP server, we might want to use a temporary file if the document is dirty
	// But let's assume we can pass the content via stdin or just check the file on disk for now
	// Since the user is likely saving.
	
	// Better: Use a dedicated check-point in the compiler that accepts source from stdin
	// For now, let's try to find quantum.exe
	const quantumPath = path.resolve(__dirname, '..', '..', 'quantum.exe');
	
	if (!fs.existsSync(quantumPath)) {
		return;
	}

	// We'll write the content to a temp file to check it
	const tempPath = localPath + '.tmp';
	fs.writeFileSync(tempPath, text);

	const command = `"${quantumPath}" --check "${tempPath}"`;

	exec(command, (error, stdout, stderr) => {
		const diagnostics: Diagnostic[] = [];
		const lines = (stdout + stderr).split('\n');

		for (const line of lines) {
			// Expected format from main.cpp checkFile:
			// path:line:col: error: message
			// path:line:1: warning: message
			const match = line.match(/.*:(\d+):(\d+)?:?\s*(error|warning):\s*(.*)/);
			if (match) {
				const lineNum = parseInt(match[1]) - 1;
				const colNum = match[2] ? parseInt(match[2]) - 1 : 0;
				const severity = match[3] === 'error' ? DiagnosticSeverity.Error : DiagnosticSeverity.Warning;
				const message = match[4];

				diagnostics.push({
					severity,
					range: {
						start: { line: lineNum, character: colNum },
						end: { line: lineNum, character: colNum + 100 } // Span the line
					},
					message,
					source: 'quantum'
				});
			}
		}

		// Send the computed diagnostics to VSCode.
		connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
		
		// Clean up temp file
		if (fs.existsSync(tempPath)) {
			fs.unlinkSync(tempPath);
		}
	});
}

connection.onDidChangeWatchedFiles(_change => {
	// Monitored files have change in VSCode
	connection.console.log('We received a file change event');
});

// This handler provides the initial list of the completion items.
connection.onCompletion(
	(_textDocumentPosition: TextDocumentPositionParams): CompletionItem[] => {
		// The pass parameter contains the position of the text document in
		// which code complete got requested. For the example we ignore this
		// info and return a fixed set of completion items.
		return [
			{
				label: 'print',
				kind: CompletionItemKind.Function,
				data: 1
			},
			{
				label: 'let',
				kind: CompletionItemKind.Keyword,
				data: 2
			},
			{
				label: 'fn',
				kind: CompletionItemKind.Keyword,
				data: 3
			}
		];
	}
);

// This handler resolves additional information for the item selected in
// the completion list.
connection.onCompletionResolve(
	(item: CompletionItem): CompletionItem => {
		if (item.data === 1) {
			item.detail = 'Print Function';
			item.documentation = 'Prints values to stdout.';
		} else if (item.data === 2) {
			item.detail = 'Variable Declaration';
			item.documentation = 'Declares a new variable.';
		}
		return item;
	}
);

connection.onHover((params) => {
	// Basic hover support
	return null;
});

// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

// Listen on the connection
connection.listen();
