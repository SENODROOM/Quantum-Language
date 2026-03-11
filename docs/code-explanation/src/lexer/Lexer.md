# Lexer() Constructor - Lexer Initialization

## Overview
The `Lexer()` constructor initializes a new lexical analyzer instance with source code text. It sets up the initial position tracking (character position, line number, and column number) that enables precise error reporting and token location information throughout the lexical analysis process.

## Complete Source Code

```cpp
Lexer::Lexer(const std::string &source)
    : src(source), pos(0), line(1), col(1) {}
```

## Line-by-Line Analysis

### **Function Signature (Line 1)**
```cpp
Lexer::Lexer(const std::string &source)
```
- **Constructor**: Special member function for object initialization
- **`const std::string &source`**: Constant reference to source code text
- **Efficient Parameter**: Reference avoids copying the entire source string
- **Explicit Constructor**: `explicit` keyword prevents implicit conversions

### **Member Initializer List (Line 2)**
```cpp
    : src(source), pos(0), line(1), col(1) {}
```

**Initialization Analysis:**
- **`src(source)`**: Copies source code string into lexer instance
- **`pos(0)`**: Sets character position to start of string (0-based)
- **`line(1)`**: Sets line number to 1 (1-based for user-friendly error reporting)
- **`col(1)`**: Sets column number to 1 (1-based for user-friendly error reporting)
- **Empty Body**: All initialization done in member initializer list

**Position Tracking Strategy:**
- **Character Position**: 0-based index into source string for internal use
- **Line/Column**: 1-based coordinates for user-facing error messages
- **Consistency**: Matches standard editor and compiler error reporting conventions

## Design Patterns and Architecture

### **RAII Pattern**
```cpp
class Lexer {
    std::string src;  // Resource acquisition
    // ... lexer operations
    // Automatic cleanup when object goes out of scope
};
```
- **Resource Acquisition**: Source code copied during construction
- **Initialization**: All member variables properly initialized
- **Automatic Cleanup**: String destructor handles memory management

### **Member Initializer List Pattern**
```cpp
Lexer::Lexer(const std::string &source)
    : src(source), pos(0), line(1), col(1) {}
```
- **Efficient Initialization**: Direct initialization vs assignment
- **Performance**: Avoids default construction followed by assignment
- **Required for Constants**: Essential for const and reference members
- **Best Practice**: Preferred C++ initialization method

### **State Initialization Pattern**
```cpp
// Initial lexer state
pos = 0;    // At beginning of source
line = 1;   // First line
col = 1;    // First column
```
- **Known Starting State**: Predictable initial conditions
- **Reset Capability**: Easy to reset lexer to initial state
- **Debugging**: Clear initial state for debugging

## Technical Considerations

### **Memory Management**
```cpp
// String copying strategy
std::string src(source);  // Copy constructor
```
- **Copy Semantics**: Source string copied into lexer instance
- **Ownership**: Lexer owns its copy of source code
- **Independence**: Original source can be modified without affecting lexer
- **Lifetime**: Source string lifetime matches lexer lifetime

### **Position Tracking**
```cpp
// Position coordinate systems
size_t pos;   // Internal: 0-based character index
int line;     // External: 1-based line number
int col;      // External: 1-based column number
```
- **Dual Systems**: Internal (0-based) and external (1-based) coordinates
- **Conversion**: Easy conversion between coordinate systems
- **User Experience**: 1-based coordinates match user expectations
- **Implementation**: 0-based indexing simplifies internal operations

### **Exception Safety**
```cpp
// Constructor exception safety
Lexer(const std::string &source) : src(source), ...
```
- **Strong Guarantee**: Either fully constructed or no construction
- **Exception Propagation**: String allocation exceptions propagate to caller
- **No Leaks**: RAII ensures cleanup even if construction fails
- **Valid State**: Object always in valid state if construction succeeds

## Usage Patterns

### **Basic Usage**
```cpp
std::string sourceCode = "print 'Hello, World!'";
Lexer lexer(sourceCode);
auto tokens = lexer.tokenize();
```

### **File Processing**
```cpp
std::ifstream file("script.sa");
std::string source((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
Lexer lexer(source);
auto tokens = lexer.tokenize();
```

### **Interactive Mode**
```cpp
std::string input = getUserInput();
Lexer lexer(input);
auto tokens = lexer.tokenize();
```

### **Error Recovery**
```cpp
try {
    std::string source = readSourceFile(filename);
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    // Process tokens...
}
catch (const std::bad_alloc&) {
    std::cerr << "Out of memory reading source file\n";
}
```

## Integration Points

### **Parser Integration**
```cpp
class Parser {
    Lexer lexer;
public:
    Parser(const std::string &source) : lexer(source) {}
    
    void parse() {
        auto tokens = lexer.tokenize();
        // Parse tokens...
    }
};
```

### **REPL Integration**
```cpp
class REPL {
    void processLine(const std::string &line) {
        Lexer lexer(line);
        auto tokens = lexer.tokenize();
        // Execute tokens...
    }
};
```

### **Testing Framework**
```cpp
void testLexicalAnalysis(const std::string &source, 
                       const std::vector<Token> &expected) {
    Lexer lexer(source);
    auto actual = lexer.tokenize();
    assertTokensEqual(expected, actual);
}
```

## Performance Considerations

### **String Copying**
```cpp
// Current approach: copy source string
src(source);  // O(n) copy operation

// Alternative: reference external string
const std::string &src;  // No copy, but lifetime management
```
- **Current Choice**: Copy for simplicity and safety
- **Memory Overhead**: One additional copy of source code
- **Performance Impact**: O(n) copy cost during construction
- **Trade-off**: Safety and simplicity vs. memory efficiency

### **Initialization Cost**
```cpp
// Construction cost analysis
Lexer lexer(source);  // O(n) for string copy
```
- **One-Time Cost**: Construction cost paid once per lexer instance
- **Amortized**: Cost spread over all tokenization operations
- **Memory Allocation**: Single allocation for string copy
- **Cache Efficiency**: Contiguous memory layout for source string

## Debugging and Troubleshooting

### **Common Issues**

#### **Empty Source**
```cpp
Lexer lexer("");  // Valid but produces only EOF token
```
- **Valid Input**: Empty string is valid input
- **Expected Behavior**: Single EOF token produced
- **Edge Case**: Handled correctly by tokenization logic

#### **Large Source Files**
```cpp
// Very large source file
std::string hugeSource = readHugeFile();  // May fail
Lexer lexer(hugeSource);  // Memory allocation may fail
```
- **Memory Limitations**: May fail for very large files
- **Alternative**: Stream-based tokenization for large files
- **Error Handling**: Handle std::bad_alloc exceptions

#### **Source Modification**
```cpp
std::string source = "original";
Lexer lexer(source);
source = "modified";  // Doesn't affect lexer
```
- **Independent Copy**: Lexer has its own copy
- **Isolation**: Source changes don't affect lexer
- **Expected Behavior**: Correct and safe isolation

### **Debugging Support**
```cpp
// Debug constructor
Lexer::Lexer(const std::string &source) 
    : src(source), pos(0), line(1), col(1) {
    if (getenv("DEBUG_LEXER")) {
        std::cout << "Lexer initialized with " 
                  << source.length() << " characters\n";
    }
}
```

## Extension Points

### **Configuration Options**
```cpp
class Lexer {
public:
    Lexer(const std::string &source, 
          const LexerConfig &config = LexerConfig{})
        : src(source), config_(config), pos(0), line(1), col(1) {}
    
private:
    LexerConfig config_;
};
```

### **Preprocessing Support**
```cpp
Lexer::Lexer(const std::string &source, bool preprocess = true)
    : src(preprocess ? preprocessSource(source) : source), 
      pos(0), line(1), col(1) {}
```

### **Memory Optimization**
```cpp
class Lexer {
public:
    Lexer(std::string &&source)  // Move constructor
        : src(std::move(source)), pos(0), line(1), col(1) {}
};
```

## Why This Design Works

### **Simplicity**
- **Clear Purpose**: Obviously initializes lexer with source code
- **Minimal Interface**: Single parameter with clear meaning
- **Self-Documenting**: Constructor name and parameter are self-explanatory
- **No Surprises**: Behavior matches user expectations

### **Reliability**
- **Exception Safe**: Strong exception guarantee
- **Resource Management**: RAII handles memory automatically
- **Valid State**: Always creates objects in valid state
- **Predictable**: Consistent behavior across all inputs

### **Performance**
- **Efficient Initialization**: Member initializer list
- **One-Time Cost**: Construction cost paid once
- **Memory Layout**: Contiguous source string storage
- **Cache Friendly**: Sequential access pattern during tokenization

### **Usability**
- **Intuitive Interface**: Simple, obvious constructor
- **Flexible**: Works with any string source
- **Error Handling**: Clear error propagation
- **Integration Ready**: Easy to use in larger systems

The `Lexer()` constructor demonstrates excellent C++ design principles, providing a simple, efficient, and reliable way to initialize lexical analyzers with proper resource management and position tracking for accurate error reporting.
