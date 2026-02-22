const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const os = require('os');

function findQuantum(workspaceFolder) {
  const candidates = [];
  if (workspaceFolder) {
    candidates.push(path.join(workspaceFolder, 'quantum.exe'));
    candidates.push(path.join(workspaceFolder, 'quantum'));
  }
  candidates.push(path.join(os.homedir(), 'AppData', 'Local', 'Programs', 'Quantum', 'quantum.exe'));
  candidates.push(path.join(process.env.LOCALAPPDATA || '', 'Programs', 'Quantum', 'quantum.exe'));
  candidates.push(path.join(process.env.LOCALAPPDATA || '', 'Microsoft', 'WindowsApps', 'quantum.cmd'));
  candidates.push('quantum');
  for (const c of candidates) {
    try {
      cp.execFileSync(c, ['--version'], { timeout: 1000 });
      return c;
    } catch {}
  }
  return null;
}

function parseDiagnostics(output, doc) {
  const diags = [];
  const lines = output.split(/\r?\n/);
  for (const line of lines) {
    let m = line.match(/^\s*(.+):(\d+):(\d+):\s*error:\s*(.+)$/i);
    if (m) {
      const ln = Math.max(0, Number(m[2]) - 1);
      const col = Math.max(0, Number(m[3]) - 1);
      const range = new vscode.Range(ln, col, ln, col + 1);
      diags.push(new vscode.Diagnostic(range, m[4], vscode.DiagnosticSeverity.Error));
      continue;
    }
    m = line.match(/ParseError.*line\s+(\d+):(\d+)/i);
    if (m) {
      const ln = Math.max(0, Number(m[1]) - 1);
      const col = Math.max(0, Number(m[2]) - 1);
      const range = new vscode.Range(ln, col, ln, col + 1);
      diags.push(new vscode.Diagnostic(range, line.trim(), vscode.DiagnosticSeverity.Error));
    }
  }
  return diags;
}

function activate(context) {
  const collection = vscode.languages.createDiagnosticCollection('quantum');
  context.subscriptions.push(collection);

  const workspaceFolder = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]?.uri.fsPath;
  const quantumPath = findQuantum(workspaceFolder);

  async function lint(doc) {
    if (!doc || doc.languageId !== 'quantum') return;
    if (!quantumPath) {
      collection.set(doc.uri, []);
      return;
    }
    try {
      const args = ['--check', doc.uri.fsPath];
      let out = '';
      try {
        out = cp.execFileSync(quantumPath, args, { encoding: 'utf8', timeout: 3000 });
      } catch (e) {
        out = (e.stdout || '') + (e.stderr || '');
        if (!/--check/.test(args.join(' '))) return;
        if (!out) {
          try {
            out = cp.execFileSync(quantumPath, [doc.uri.fsPath], { encoding: 'utf8', timeout: 3000 });
          } catch (ee) {
            out = (ee.stdout || '') + (ee.stderr || '');
          }
        }
      }
      const diags = parseDiagnostics(out, doc);
      collection.set(doc.uri, diags);
    } catch {
      collection.set(doc.uri, []);
    }
  }

  context.subscriptions.push(vscode.workspace.onDidOpenTextDocument(lint));
  context.subscriptions.push(vscode.workspace.onDidSaveTextDocument(lint));
  context.subscriptions.push(vscode.workspace.onDidChangeTextDocument(e => lint(e.document)));
  vscode.workspace.textDocuments.forEach(lint);
}

function deactivate() {}

module.exports = { activate, deactivate };
