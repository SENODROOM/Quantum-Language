# current() Function - Current Character Access

## Overview
The `current()` function provides immediate access to the character at the lexer's current position in the source code. It serves as a fundamental building block for lexical analysis, enabling the lexer to examine characters during token recognition while maintaining bounds safety and consistent behavior.

## Complete Source Code

```cpp
char Lexer::current() const
{
    return pos < src.size() ? src[pos] : '\0';
```

## Line-by-Line Analysis

### **Function Signature (Line 1)**
```cpp
char Lexer::current() const
```
- **`char`**: Returns a single character
- **`Lexer::current()`**: Member function of Lexer class
- **`const`**: Read-only function, doesn't modify lexer state
- **No Parameters**: Uses current lexer position automatically

### **Bounds-Safe Access (Line 3)**
```cpp
return pos < src.size() ? src[pos] : '\0';
```

**Conditional Expression Analysis:**
- **`pos < src.size()`**: Checks if position is within source string bounds
- **`src[pos]`**: Returns character at current position if in bounds
- **`'\0'`**: Returns null terminator if position is past end of source
- **Ternary Operator**: Compact conditional expression for bounds checking

**Safety Features:**
- **No Undefined Behavior**: Never accesses array out of bounds
- **Consistent Return**: Always returns a valid character value
- **Null Terminator**: Standard C-style end-of-input indicator
- **Predictable Behavior**: Same result for any out-of-bounds position

## Design Patterns and Architecture

### **Bounds Checking Pattern**
```cpp
char current() const {
    return (pos < src.size()) ? src[pos] : '\0';
}
```
- **Defensive Programming**: Always checks array bounds
- **Fail-Safe Behavior**: Returns safe default value
- **No Exceptions**: Avoids throwing for out-of-bounds access
- **Consistent Interface**: Always returns valid char

### **Accessor Pattern**
```cpp
class Lexer {
private:
    size_t pos;
    std::string src;
public:
    char current() const { /* safe access */ }
};
```
- **Encapsulation**: Private data, public accessor
- **Read-Only Access**: Const method prevents modification
- **Abstraction**: Hides implementation details
- **Safe Interface**: Provides safe access to internal state

### **Null Object Pattern**
```cpp
// Returns '\0' instead of throwing for end-of-input
return pos < src.size() ? src[pos] : '\0';
```
- **Null Return Value**: '\0' indicates end of input
- **Graceful Degradation**: No exception throwing
- **Predictable Behavior**: Always returns valid character
- **Standard Convention**: '\0' is standard string terminator

## Technical Considerations

### **Performance Characteristics**
```cpp
// Time complexity: O(1) - direct array access
// Space complexity: O(1) - no additional memory
```
- **Constant Time**: Direct array indexing
- **No Overhead**: Minimal function call overhead
- **Inline Optimization**: Likely inlined by compiler
- **Cache Friendly**: Sequential memory access pattern

### **Bounds Safety**
```cpp
// Safe vs unsafe access
char safe = current();           // Always safe
char unsafe = src[pos];          // Potentially unsafe
```
- **Safe Alternative**: Prevents buffer overflows
- **Undefined Behavior**: Avoids undefined behavior
- **Security**: Prevents memory access violations
- **Reliability**: Consistent behavior in all cases

### **Character Encoding**
```cpp
// Works with any single-byte encoding
return src[pos];  // Direct byte access
```
- **Byte-Level Access**: Works with UTF-8 and ASCII
- **Encoding Agnostic**: Doesn't interpret character encoding
- **Multi-Byte Support**: Caller handles multi-byte character processing
- **Flexibility**: Works with any 8-bit encoding

## Usage Patterns

### **Token Recognition**
```cpp
char c = current();
if (std::isdigit(c)) {
    // Start reading number token
}
```

### **Operator Detection**
```cpp
if (current() == '+') {
    advance();
    if (current() == '+') {
        // Found ++ operator
    }
}
```

### **String Literal Processing**
```cpp
if (current() == '"') {
    // Start reading string literal
    advance();
    while (current() != '"') {
        // Process string content
        advance();
    }
}
```

### **Error Detection**
```cpp
if (current() == '\0') {
    // End of input reached
    throw QuantumError("LexError", "Unexpected end of input", line);
}
```

## Integration Points

### **With advance() Function**
```cpp
char c = current();  // Look at current character
advance();           // Move to next character
```

### **With peek() Function**
```cpp
char curr = current();  // Current character
char next = peek();     // Look ahead character
```

### **In Token Recognition Loops**
```cpp
while (pos < src.size()) {
    char c = current();
    switch (c) {
        case '(': // Handle opening parenthesis
        case ')': // Handle closing parenthesis
        // ... other cases
    }
    advance();
}
```

## Error Handling and Edge Cases

### **End of Input Handling**
```cpp
char c = current();
if (c == '\0') {
    // Reached end of source code
    return TokenType::EOF_TOKEN;
}
```

### **Empty Source String**
```cpp
Lexer lexer("");  // Empty source
char c = lexer.current();  // Returns '\0'
```

### **Position at End**
```cpp
// After reading last character
char c = current();  // Returns '\0'
```

## Performance Optimization

### **Inline Optimization**
```cpp
// Compiler likely inlines this simple function
inline char Lexer::current() const {
    return pos < src.size() ? src[pos] : '\0';
}
```

### **Branch Prediction**
```cpp
// Most calls will be in bounds
return pos < src.size() ? src[pos] : '\0';
// Branch predictor optimizes for true condition
```

### **Cache Locality**
```cpp
// Access pattern: sequential through source string
for (; pos < src.size(); advance()) {
    char c = current();  // Good cache locality
}
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Infinite Loop Prevention**
```cpp
while (current() != '\0') {  // Proper loop termination
    // Process character
    advance();  // Must advance to avoid infinite loop
}
```

#### **Position Validation**
```cpp
char c = current();
if (c == '\0') {
    // Handle end of input gracefully
    break;
}
```

### **Debugging Support**
```cpp
char Lexer::current() const {
    char result = pos < src.size() ? src[pos] : '\0';
    
    if (getenv("DEBUG_LEXER")) {
        std::cout << "current(): pos=" << pos 
                  << " char='" << result << "' (" 
                  << (int)result << ")\n";
    }
    
    return result;
}
```

## Extension Points

### **Unicode Support**
```cpp
char32_t currentUnicode() const {
    if (pos >= src.size()) return U'\0';
    
    // Simple UTF-8 handling
    char32_t codepoint = 0;
    if ((src[pos] & 0x80) == 0) {
        codepoint = src[pos];
    } else if ((src[pos] & 0xE0) == 0xC0) {
        // 2-byte UTF-8
        if (pos + 1 < src.size()) {
            codepoint = ((src[pos] & 0x1F) << 6) | (src[pos+1] & 0x3F);
        }
    }
    // ... more UTF-8 handling
    return codepoint;
}
```

### **Position Validation**
```cpp
char current() const {
    assert(pos <= src.size());  // Debug assertion
    return pos < src.size() ? src[pos] : '\0';
}
```

### **Error Reporting Enhancement**
```cpp
char current() const {
    if (pos >= src.size()) {
        lastError_ = "Attempted to read past end of source";
        return '\0';
    }
    return src[pos];
}
```

## Why This Design Works

### **Simplicity**
- **Clear Purpose**: Obviously returns current character
- **Minimal Implementation**: Single line of logic
- **Self-Documenting**: Function name explains behavior
- **No Surprises**: Predictable behavior in all cases

### **Safety**
- **Bounds Checking**: Never accesses invalid memory
- **Consistent Return**: Always returns valid character
- **No Exceptions**: Safe to call in any context
- **Undefined Behavior Free**: No undefined behavior possible

### **Performance**
- **Constant Time**: O(1) direct array access
- **Minimal Overhead**: Simple conditional expression
- **Cache Friendly**: Sequential access pattern
- **Inline Optimization**: Likely inlined by compiler

### **Reliability**
- **Deterministic**: Same input always produces same output
- **Thread Safe**: Read-only function, safe for concurrent access
- **Exception Safe**: Never throws exceptions
- **Well-Defined**: Clear behavior for all input conditions

The `current()` function demonstrates excellent utility design, providing a simple, safe, and efficient way to access characters during lexical analysis while maintaining bounds safety and consistent behavior across all edge cases.
