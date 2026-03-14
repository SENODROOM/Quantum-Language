# Implementation Details

## 🔧 Core Implementation

### Build System
- **CMake 3.14+** - Cross-platform build configuration
- **C++17 Standard** - Modern C++ features
- **MSVC/GCC/Clang** - Multi-compiler support
- **Release Optimization** - `-O2` for performance

### Project Structure
```
Quantum-Language/
├── src/                    # Source files
│   ├── main.cpp           # Entry point & CLI
│   ├── Lexer.cpp          # Lexical analysis
│   ├── Parser.cpp         # Syntax analysis
│   ├── Interpreter.cpp    # Runtime execution
│   ├── Value.cpp          # Value system
│   ├── Token.cpp          # Token definition
│   └── TypeChecker.cpp    # Semantic analysis
├── include/               # Header files
│   ├── AST.h              # AST node definitions
│   ├── Lexer.h            # Lexer interface
│   ├── Parser.h           # Parser interface
│   ├── Interpreter.h      # Interpreter interface
│   ├── Value.h            # Value system
│   ├── Token.h            # Token types
│   └── TypeChecker.h      # Type checker
├── examples/              # Example programs
├── tests/                 # Test files
└── docs/                  # Documentation
```

## 🔤 Lexer Implementation

### Token Types
```cpp
enum class TokenType {
    // Literals
    NUMBER, STRING, BOOL, NIL,
    
    // Identifiers & Keywords
    IDENTIFIER, IF, ELSE, WHILE, FOR, 
    FUNCTION, CLASS, RETURN, LET, VAR, CONST,
    
    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    PLUS_PLUS, MINUS_MINUS,
    
    // Comparison
    EQ, EQ_EQ, BANG_EQ, LT, LTE, GT, GTE,
    
    // Logical
    AND, OR, NOT, AMP_AMP, PIPE_PIPE,
    
    // Assignment
    EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL,
    
    // Punctuation
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, DOT, COLON, SEMICOLON, ARROW,
    
    // Special
    NEWLINE, INDENT, DEDENT, EOF
};
```

### Lexing Strategy
1. **Character-by-character scanning**
2. **Number recognition** (integers, floats, scientific notation)
3. **String handling** (escape sequences, interpolation)
4. **Keyword detection** (identifier lookup table)
5. **Multi-character operators** (>=, ==, !=, etc.)

### Code Example
```cpp
Token Lexer::scanNumber() {
    size_t start = current;
    
    while (isdigit(peek())) advance();
    
    if (peek() == '.' && isdigit(peekNext())) {
        advance(); // Consume decimal point
        while (isdigit(peek())) advance();
    }
    
    double value = std::stod(source.substr(start, current - start));
    return Token(TokenType::NUMBER, value, line, column);
}
```

## 🌳 Parser Implementation

### Parsing Algorithm
- **Recursive Descent Parser**
- **Top-down parsing strategy**
- **Error recovery** with synchronization
- **Precedence climbing** for expressions

### Grammar Rules (Simplified)
```
program        → statement* EOF

statement      → exprStmt | varDecl | funcDecl | classDecl
               | ifStmt | whileStmt | forStmt | returnStmt
               | printStmt | inputStmt

varDecl        → ("let"|"var"|"const") identifier ("=" expression)?

funcDecl       → "function" identifier "(" parameters? ")" block

classDecl      → "class" identifier (":" identifier)? block

ifStmt         → "if" "(" expression ")" statement 
                ("else" "if" "(" expression ")" statement)*
                ("else" statement)?

whileStmt      → "while" "(" expression ")" statement

forStmt        → "for" "(" (varDecl | exprStmt | ";") 
                expression? ";" expression? ")" statement

expression     → assignment

assignment     → identifier ("=" | "+=" | "-=" | "*=" | "/=") assignment
               | ternary

ternary        → logical_or ("?" expression ":" expression)?

logical_or     → logical_and ("or" logical_and)*

logical_and    → equality ("and" equality)*

equality       → comparison (("==" | "!=") comparison)*

comparison     → term (("<" | ">" | "<=" | ">=") term)*

term           → factor (("+" | "-") factor)*

factor         → unary (("*" | "/" | "%") unary)*

unary          → ("-" | "!" | "not" | "++" | "--") unary
               | primary

primary        → NUMBER | STRING | BOOL | NIL | "this"
               | "(" expression ")" | identifier
               | "super" "." identifier
               | lambdaExpr | arrayLiteral | dictLiteral
```

### AST Node Construction
```cpp
std::unique_ptr<BinaryExpr> Parser::parseBinaryExpr(
    std::unique_ptr<Expr> left, 
    Token op, 
    std::unique_ptr<Expr> right
) {
    auto expr = std::make_unique<BinaryExpr>();
    expr->op = op.lexeme;
    expr->left = std::move(left);
    expr->right = std::move(right);
    return expr;
}
```

## 🎯 Value System Implementation

### Value Storage
```cpp
using QuantumValue = std::variant<
    QuantumNil,           // null/undefined
    bool,                // boolean
    double,              // number
    std::string,         // string
    std::shared_ptr<Array>,   // array
    std::shared_ptr<Dict>,    // object/dict
    std::shared_ptr<QuantumFunction>,  // function
    std::shared_ptr<QuantumNative>,     // native function
    std::shared_ptr<QuantumInstance>,   // class instance
    std::shared_ptr<QuantumClass>,      // class definition
    std::shared_ptr<QuantumPointer>     // C++ pointer
>;
```

### Type Checking
```cpp
class QuantumValue {
public:
    bool isNumber() const { return std::holds_alternative<double>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isBool() const { return std::holds_alternative<bool>(data); }
    bool isNil() const { return std::holds_alternative<QuantumNil>(data); }
    bool isArray() const { return std::holds_alternative<std::shared_ptr<Array>>(data); }
    bool isDict() const { return std::holds_alternative<std::shared_ptr<Dict>>(data); }
    
    double asNumber() const { return std::get<double>(data); }
    const std::string& asString() const { return std::get<std::string>(data); }
    bool asBool() const { return std::get<bool>(data); }
};
```

### Memory Management
- **Reference Counting**: `std::shared_ptr` for garbage collection
- **RAII**: Automatic cleanup
- **Circular Reference Handling**: Weak references where needed

## 🚀 Interpreter Implementation

### Environment System
```cpp
class Environment {
private:
    std::unordered_map<std::string, QuantumValue> values;
    std::shared_ptr<Environment> enclosing;
    
public:
    void define(const std::string& name, const QuantumValue& value) {
        values[name] = value;
    }
    
    QuantumValue get(const std::string& name) {
        auto it = values.find(name);
        if (it != values.end()) return it->second;
        if (enclosing) return enclosing->get(name);
        throw RuntimeError("Undefined variable: " + name);
    }
};
```

### Function Call Handling
```cpp
QuantumValue Interpreter::callFunction(
    std::shared_ptr<QuantumFunction> fn, 
    std::vector<QuantumValue> args
) {
    // Create new environment for function scope
    auto funcEnv = std::make_shared<Environment>(fn->closure);
    
    // Bind parameters
    for (size_t i = 0; i < fn->params.size(); i++) {
        if (i < args.size()) {
            funcEnv->define(fn->params[i], args[i]);
        } else if (i < fn->defaultArgs.size()) {
            funcEnv->define(fn->params[i], evaluate(*fn->defaultArgs[i]));
        }
    }
    
    // Execute function body
    std::shared_ptr<Environment> previous = env;
    env = funcEnv;
    
    try {
        execute(*fn->body);
    } catch (ReturnValue& ret) {
        env = previous;
        return ret.value;
    }
    
    env = previous;
    return QuantumValue(); // Default return value
}
```

### Method Dispatch
```cpp
QuantumValue Interpreter::callMethod(
    QuantumValue& obj, 
    const std::string& method, 
    std::vector<QuantumValue> args
) {
    if (obj.isArray()) {
        return callArrayMethod(obj.asArray(), method, args);
    } else if (obj.isString()) {
        return callStringMethod(obj.asString(), method, args);
    } else if (obj.isDict()) {
        return callDictMethod(obj.asDict(), method, args);
    } else if (auto instance = std::get_if<std::shared_ptr<QuantumInstance>>(&obj.data)) {
        return callInstanceMethod(*instance, method, args);
    }
    
    throw RuntimeError("Object has no method '" + method + "'");
}
```

## 🔧 Built-in Functions

### Registration System
```cpp
void Interpreter::registerNatives() {
    // I/O functions
    globals->define("print", QuantumValue(std::make_shared<QuantumNative>(
        "print", [](std::vector<QuantumValue> args) -> QuantumValue {
            for (size_t i = 0; i < args.size(); i++) {
                if (i > 0) std::cout << " ";
                std::cout << args[i].toString();
            }
            std::cout << std::endl;
            return QuantumValue();
        }
    )));
    
    // Mathematical functions
    globals->define("sqrt", QuantumValue(std::make_shared<QuantumNative>(
        "sqrt", [](std::vector<QuantumValue> args) -> QuantumValue {
            if (args.size() != 1 || !args[0].isNumber()) {
                throw RuntimeError("sqrt() expects one number argument");
            }
            return QuantumValue(std::sqrt(args[0].asNumber()));
        }
    )));
}
```

### String Methods
```cpp
QuantumValue Interpreter::callStringMethod(
    const std::string& str, 
    const std::string& method, 
    std::vector<QuantumValue> args
) {
    if (method == "len") {
        return QuantumValue(static_cast<double>(str.length()));
    } else if (method == "upper") {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return QuantumValue(result);
    } else if (method == "slice") {
        if (args.size() < 2) throw RuntimeError("slice() needs start and end");
        int start = static_cast<int>(args[0].asNumber());
        int end = static_cast<int>(args[1].asNumber());
        return QuantumValue(str.substr(start, end - start));
    }
    // ... more methods
}
```

## 🐛 Error Handling

### Exception Hierarchy
```cpp
class QuantumException : public std::exception {
protected:
    std::string message;
    int line;
    int column;
    
public:
    QuantumException(const std::string& msg, int line = 0, int column = 0)
        : message(msg), line(line), column(column) {}
    
    const char* what() const noexcept override { return message.c_str(); }
};

class ParseError : public QuantumException {
public:
    ParseError(const std::string& msg, int line, int column)
        : QuantumException("Parse Error: " + msg, line, column) {}
};

class RuntimeError : public QuantumException {
public:
    RuntimeError(const std::string& msg, int line = 0, int column = 0)
        : QuantumException("Runtime Error: " + msg, line, column) {}
};

class TypeError : public RuntimeError {
public:
    TypeError(const std::string& msg, int line = 0, int column = 0)
        : RuntimeError("Type Error: " + msg, line, column) {}
};
```

### Error Reporting
```cpp
void Parser::error(Token token, const std::string& message) {
    std::string msg = "[line " + std::to_string(token.line) + "] Error";
    
    if (token.type == TokenType::EOF) {
        msg += " at end";
    } else if (token.type == TokenType::IDENTIFIER) {
        msg += " at '" + token.lexeme + "'";
    } else {
        msg += " at '" + token.lexeme + "'";
    }
    
    msg += ": " + message;
    throw ParseError(msg, token.line, token.column);
}
```

## ⚡ Performance Optimizations

### String Interning
```cpp
class StringInterner {
private:
    static std::unordered_set<std::string> interned_strings;
    
public:
    static const std::string& intern(const std::string& str) {
        auto result = interned_strings.insert(str);
        return *result.first;
    }
};
```

### Function Caching
```cpp
class FunctionCache {
private:
    std::unordered_map<std::string, std::shared_ptr<QuantumNative>> native_cache;
    
public:
    std::shared_ptr<QuantumNative> getNative(const std::string& name) {
        auto it = native_cache.find(name);
        if (it != native_cache.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void setNative(const std::string& name, std::shared_ptr<QuantumNative> native) {
        native_cache[name] = native;
    }
};
```

### Inline Functions
Critical path functions marked `inline` for performance:
- Value type checks (`isNumber()`, `isString()`, etc.)
- Token creation and access
- Environment lookups

## 🔍 Debugging Features

### AST Printing
```cpp
class ASTPrinter {
public:
    std::string print(ASTNode& node) {
        return node.accept(*this);
    }
    
    std::string visit(BinaryExpr& expr) {
        return parenthesize(expr.op, *expr.left, *expr.right);
    }
    
private:
    std::string parenthesize(const std::string& name, ASTNode&... exprs) {
        std::string result = "(" + name;
        ((result += " " + print(exprs)), ...);
        result += ")";
        return result;
    }
};
```

### Runtime Tracing
```cpp
#ifdef DEBUG
#define TRACE_EXECUTION() \
    std::cout << "Executing: " << node.toString() << std::endl;
#else
#define TRACE_EXECUTION()
#endif
```

---

*This document provides detailed implementation specifics for developers working on the Quantum Language interpreter.*
