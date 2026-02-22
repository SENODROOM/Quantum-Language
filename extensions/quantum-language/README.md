# Quantum Language VS Code Extension

Provides syntax highlighting and basic language configuration for Quantum `.sa` files.

## Install Locally

1. Open this folder in a terminal:
   - `cd extensions/quantum-language`
2. Package into a `.vsix` (requires Node.js):
   - `npm i -g vsce`
   - `vsce package`
3. Install into VS Code:
   - `code --install-extension quantum-language-0.0.1.vsix`
4. Open any `.sa` file and enjoy highlighting.

## Features

- Comments with `#`
- Strings, numbers, operators
- Keywords: `let`, `const`, `fn`, `if/elif/else`, `while`, `for`, `in`, `break`, `continue`, `return`, `import`
- Built-ins like `print`, `input`, `len`, `type`, `range`, `rand`, `num`, `str`, `bool`, etc.
