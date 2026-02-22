# ⚡ Quantum Language

**Quantum** is a dynamically-typed, cybersecurity-ready scripting language written in C++17.  
Files use the `.sa` extension and run with the single command: `quantum file.sa`

```
  ██████╗ ██╗   ██╗ █████╗ ███╗   ██╗████████╗██╗   ██╗███╗   ███╗
 ██╔═══██╗██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██║   ██║████╗ ████║
 ██║   ██║██║   ██║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║
 ██║▄▄ ██║██║   ██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║
 ╚██████╔╝╚██████╔╝██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║
  ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝
```

---

## 🚀 Quick Start

### Build
```bash
make              # compile
sudo make install # install to /usr/local/bin
```

### Run
```bash
quantum hello.sa         # run a script
quantum                  # start REPL
quantum --help           # show help
```

---

## 📖 Language Reference

### Variables
No type declarations — works like Python:
```quantum
let x = 42
let name = "Alice"
let active = true
let nothing = nil
const MAX = 100      # immutable constant
```

### Functions
JS-style declaration syntax:
```quantum
fn greet(name) {
    return "Hello, " + name + "!"
}

# Lambda / anonymous function
let square = fn(n) { return n * n }

# Closures
fn make_adder(n) {
    return fn(x) { return x + n }
}
let add5 = make_adder(5)
print(add5(10))  # 15
```

### Control Flow
```quantum
if x > 10 {
    print("big")
} elif x > 0 {
    print("small")
} else {
    print("negative")
}

while count < 10 {
    count += 1
}

for item in [1, 2, 3] {
    print(item)
}

for i in range(10) {
    if i == 5 { break }
}
```

### Arrays
```quantum
let arr = [1, 2, 3, 4, 5]
arr.push(6)
arr.pop()
arr.slice(1, 3)
arr.map(fn(x) { return x * 2 })
arr.filter(fn(x) { return x > 2 })
arr.sort()
arr.reverse()
arr.join(", ")
arr.contains(3)
```

### Dictionaries
```quantum
let user = {"name": "Alice", "role": "admin"}
user["key"] = "value"
user.get("key", "default")
user.has("name")
user.keys()
user.values()
user.delete("key")
```

### String Methods
```quantum
"hello".upper()          # HELLO
"HELLO".lower()          # hello
"  hi  ".trim()          # hi
"a,b,c".split(",")       # ["a","b","c"]
"hello world".replace("world", "quantum")
"hello".contains("ell")  # true
"hello".starts_with("he")
"hello".ends_with("lo")
"hello".slice(1, 3)      # el
"ha".repeat(3)           # hahaha
"hello".chars()          # ["h","e","l","l","o"]
```

### Operators
```quantum
# Arithmetic
+ - * / % **          # power: 2 ** 10 = 1024

# Comparison
== != < > <= >=

# Logical
and  or  not

# Bitwise (key for cybersecurity)
& | ^ ~ << >>

# Assignment
= += -= *= /=
```

### Built-in Functions
```quantum
# Type conversion
num("3.14")    str(42)    bool(0)    type(x)

# Math
abs(x)   sqrt(x)   floor(x)   ceil(x)   round(x)
pow(x,y) log(x)    sin(x)     cos(x)    tan(x)
min(a,b) max(a,b)

# String/Bytes
chr(65)       # "A"
ord("A")      # 65
hex(255)      # "0xFF"
bin(8)        # "0b1000"

# Collections
len(x)
range(start, stop, step)
array(size, fill)
keys(dict)
values(dict)

# Random
rand()           # 0.0 to 1.0
rand(lo, hi)     # float in range
rand_int(lo, hi)

# Cybersecurity
xor_bytes(data, key)    # XOR two strings
to_hex(s)               # string to hex
from_hex(h)             # hex to string
rot13(s)                # ROT13 cipher
base64_encode(s)        # base64 encoding

# Utility
time()           # unix timestamp (ms)
exit(code)       # exit program
assert(cond, msg)
```

### Constants
```quantum
PI    # 3.14159...
E     # 2.71828...
INF   # infinity
```

---

## 🔐 Cybersecurity Features

Quantum is designed to grow into a full cybersecurity language. Current built-ins:

| Function | Description |
|----------|-------------|
| `xor_bytes(a, b)` | XOR two byte strings |
| `to_hex(s)` | Convert string to hex |
| `from_hex(h)` | Decode hex string |
| `rot13(s)` | ROT13 cipher |
| `base64_encode(s)` | Base64 encode |
| `hex(n)` | Number to hex string |
| `bin(n)` | Number to binary string |
| `ord(c)` / `chr(n)` | Char/byte conversions |
| Bitwise ops `& \| ^ ~ << >>` | Bit manipulation |

**Reserved keywords for future modules:**
- `scan` — network scanning
- `payload` — payload crafting
- `encrypt` / `decrypt` — symmetric crypto
- `hash` — cryptographic hashing

---

## 🏗️ Architecture

```
quantum/
├── include/
│   ├── Token.h          # Token types and structure
│   ├── Lexer.h          # Tokenizer
│   ├── AST.h            # AST node variants (C++17 std::variant)
│   ├── Parser.h         # Recursive descent parser
│   ├── Value.h          # Runtime value type + Environment
│   ├── Interpreter.h    # Tree-walk interpreter
│   └── Error.h          # Error hierarchy
├── src/
│   ├── main.cpp         # CLI entry point + REPL
│   ├── Token.cpp
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   ├── Value.cpp
│   └── Interpreter.cpp
├── examples/
│   ├── hello.sa         # Hello World
│   ├── features.sa      # Full language showcase
│   ├── cybersec.sa      # Cybersecurity tools
│   └── advanced.sa      # Advanced patterns
├── Makefile
├── CMakeLists.txt
└── install.sh
```

### Design Principles
- **OOP architecture** — Every major component is a class
- **C++17 `std::variant`** — Type-safe AST without inheritance overhead
- **Closures** — Full lexical scoping via shared `Environment` pointers
- **Scalable** — Add new token types, AST nodes, and built-ins without breaking existing code
- **Error hierarchy** — Typed exceptions for clean error reporting

---

## 🔮 Roadmap

- [ ] Module/import system (`import net`, `import crypto`)
- [ ] Class system with inheritance
- [ ] Try/catch exception handling
- [ ] Socket API for network scanning
- [ ] AES/RSA encryption built-ins
- [ ] SHA-256/MD5 hashing
- [ ] Regex support
- [ ] File I/O (`file.read()`, `file.write()`)
- [ ] Standard library modules
- [ ] Bytecode compiler + VM (performance upgrade)
- [ ] REPL tab-completion

---

## 📄 License
MIT License — built for learning, hacking, and growing.
