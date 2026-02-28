# Quantum Language VS Code Extension v1.1.0

Syntax highlighting, multi-syntax coloring, auto-completion, hover docs, signature help, and code snippets for Quantum Language (`.sa` files).

## Features

- 🟢 **Green Comments** — All `#` and `/* */` comments render in green across all syntax modes
- 🎨 **Per-line Multi-syntax Highlighting** — Tag any line with `#Python`, `#C`, `#C++`, or `#JS` to apply that language's color theme
- 🔮 **Smart Auto-Completion** — Keywords, all built-in functions, contextual completions after `let`, `fn`, `def`, `class`, and `.`
- 📖 **Hover Documentation** — Hover over any built-in to see its signature and description
- ✍️ **Signature Help** — Argument hints appear as you type inside function calls
- 📝 **40+ Code Snippets** — Templates for functions, loops, classes, XOR encryption, FizzBuzz, HOF pipelines, and more
- 🔐 **Cybersecurity Keywords** — `scan`, `payload`, `encrypt`, `decrypt`, `hash` highlighted distinctly

## Multi-syntax Line Tags

Add a tag at the end of any line to force that language's color scheme for that line only:

```quantum
print("Hello")                              #Python
printf("Score: %d\n", score);              #C
cout << "Value: " << val << endl;          #C++
console.log("Debug:", val)                 #JS
```

Supported tags: `#Python`, `#C`, `#C++`, `#JS`, `#JavaScript`

## Installation

### From VSIX (recommended)

1. Open VS Code
2. Press `Ctrl+Shift+P` → type `Extensions: Install from VSIX...`
3. Select `quantum-language-1.1.0.vsix`
4. Reload VS Code

### From Source (Development)

```bash
cd quantum-extension
npm install
npm run compile
# Press F5 in VS Code to launch Extension Development Host
```

## Snippets Reference

| Prefix | Description |
|---|---|
| `fn` | Quantum function `fn name(args) { }` |
| `def` | Python-style function |
| `function` | JavaScript-style function |
| `arrow` | Arrow function `(args) => expr` |
| `arrowfn` | Arrow function with block body |
| `anon` | Anonymous function |
| `if` | If statement (brace style) |
| `ifp` | If statement (Python style) |
| `ifelse` | If-else statement |
| `ifelifelse` | Full if-elif-else (Python style) |
| `while` | While loop |
| `for` | For-in loop |
| `forrange` | `for i in range(n)` |
| `forrange2` | `for i in range(a, b)` |
| `class` | Class definition |
| `classext` | Class with inheritance |
| `closure` | Closure/counter pattern |
| `xor` | XOR encrypt/decrypt template |
| `b64` | Base64 encode template |
| `hexdump` | Hex dump template |
| `fib` | Fibonacci (recursive) |
| `fizzbuzz` | FizzBuzz |
| `hof` | Filter → Map → Reduce pipeline |
| `bitwise` | Bitwise operations demo |
| `print` | `print(value)` |
| `printf` | `printf("%s\n", value)` |
| `cout` | `cout << value << endl` |
| `tagpy` | Tag line as `#Python` |
| `tagc` | Tag line as `#C` |
| `tagcpp` | Tag line as `#C++` |
| `tagjs` | Tag line as `#JS` |
| `region` | Foldable code region |
| `assert` | `assert(cond, msg)` |
| `fmt` | Format string |
