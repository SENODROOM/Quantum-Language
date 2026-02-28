# Quantum Language VS Code Extension

Provides syntax highlighting, auto-completion, and snippets for Quantum Language (.sa files).

## Features

- 🎨 **Syntax Highlighting** for all Quantum Language constructs
- 🌐 **Per-line Multilingual Highlighting** using trailing tags (`#Python`, `#C`, `#C++`)
- 🔮 **Auto-Completion** for keywords, functions, and variables
- 📝 **Code Snippets** for common patterns
- 🛡️ **Cybersecurity Keywords** highlighting
- 🔧 **Language Configuration** for proper bracket matching

## Multilingual Line Tags

Use a line-end tag to force language-specific coloring for that line:

```quantum
print(kutta + bandru + kaka)               #Python
printf("%s %s %s\n", kutta, bandru, kaka); #C
cout << kutta << bandru << kaka;           #C++
```

## Installation

1. Clone this repository
2. Run `npm install` in the extension directory
3. Press F5 to open a new Extension Development Host window
4. Open a `.sa` file to test the extension

## Snippets

- `let` - Variable declaration
- `const` - Constant declaration  
- `fn` - Function definition
- `if` - If statement
- `ifelse` - If-else statement
- `while` - While loop
- `for` - For-in loop
- `print` - Print statement
- `input` - Input statement

## Development

```bash
npm install
npm run compile
```
