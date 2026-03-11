# peek() Function - Lookahead Character Access

## Overview
The `peek()` function provides non-destructive lookahead capability, allowing the lexer to examine characters ahead of the current position without advancing the lexer's position. This is essential for recognizing multi-character tokens (like `++`, `==`, `===`) and making parsing decisions without consuming input.

## Complete Source Code

```cpp
char Lexer::peek(int offset) const
{
    size_t p = pos + offset;
    return p < src.size() ? src[p] : '\0';
}
```

## Line-by-Line Analysis

### **Function Signature (Line 1)**
```cpp
char Lexer::peek(int offset) const
```
- **`char`**: Returns a single character
- **`Lexer::peek()`**: Member function of Lexer class
- **`int offset`**: Number of characters to look ahead (default = 1)
- **`const`**: Read-only function, doesn't modify lexer state

### **Position Calculation (Line 3)**
```cpp
size_t p = pos + offset;
```
- **`size_t p`**: Temporary variable for calculated position
- **`pos + offset`**: Adds offset to current position
- **Type Conversion**: Handles signed to unsigned conversion safely
- **Bounds Preparation**: Prepares for bounds checking

### **Bounds-Safe Access (Line 4)**
```cpp
return p < src.size() ? src[p] : '\0';
```

**Conditional Expression Analysis:**
- **`p < src.size()`**: Checks if calculated position is within bounds
- **`src[p]`**: Returns character at lookahead position if valid
- **`'\0'`**: Returns null terminator if position is out of bounds
- **Ternary Operator**: Compact conditional expression for safety

**Safety Features:**
- **No Position Change**: Current position remains unchanged
- **Bounds Checking**: Prevents array out-of-bounds access
- **Consistent Return**: Always returns valid character
- **Flexible Lookahead**: Can peek any number of characters ahead

## Design Patterns and Architecture

### **Lookahead Pattern**
```cpp
char next = peek();  // Look ahead without advancing
if (next == '+') {
    advance();      // Only advance if needed
    if (peek() == '+') {
        // Found ++ operator
    }
}
```
- **Non-Destructive**: Examines input without consuming it
- **Conditional Advancement**: Advances only when necessary
- **Multi-Token Recognition**: Enables complex token detection
- **Parsing Decisions**: Makes informed parsing choices

### **Immutable State Pattern**
```cpp
char peek(int offset) const {
    // Doesn't modify any member variables
    return calculated_character;
}
```
- **Read-Only Access**: Const method guarantees no state modification
- **Predictable Behavior**: Multiple calls return same results
- **Safe Concurrency**: Thread-safe for read operations
- **Functional Style**: Pure function behavior

### **Bounds Safety Pattern**
```cpp
size_t p = pos + offset;
return p < src.size() ? src[p] : '\0';
```
- **Defensive Programming**: Always checks array bounds
- **Graceful Degradation**: Returns safe default for out-of-bounds
- **No Exceptions**: Avoids throwing for invalid positions
- **Standard Convention**: '\0' indicates end-of-input

## Technical Considerations

### **Performance Characteristics**
```cpp
// Time complexity: O(1) - direct array access
// Space complexity: O(1) - minimal temporary storage
```
- **Constant Time**: Direct array indexing with offset
- **No Memory Allocation**: Uses stack variable only
- **Cache Friendly**: Sequential memory access pattern
- **Branch Prediction**: Optimized for in-bounds access

### **Offset Handling**
```cpp
// Various offset scenarios
char next = peek(1);        // Look ahead 1 character
char next2 = peek(2);       // Look ahead 2 characters
char prev = peek(-1);       // Look behind 1 character
char current = peek(0);     // Same as current()
```
- **Flexible Range**: Supports positive, zero, and negative offsets
- **Negative Offsets**: Can look at previous characters
- **Zero Offset**: Equivalent to current() function
- **Bounds Safe**: All offsets handled safely

### **Type Safety**
```cpp
size_t p = pos + offset;  // Safe conversion
```
- **Signed to Unsigned**: Handles int to size_t conversion
- **Overflow Protection**: Size_t large enough for valid offsets
- **Underflow Handling**: Negative offsets handled correctly
- **Well-Defined Behavior**: Clear behavior for all offset values

## Usage Patterns

### **Multi-Character Operators**
```cpp
if (current() == '+') {
    advance();
    if (peek() == '+') {
        advance();
        return Token(TokenType::PLUS_PLUS, "++", line, col);
    }
    return Token(TokenType::PLUS, "+", line, col);
}
```

### **Assignment Operators**
```cpp
if (current() == '=') {
    advance();
    if (peek() == '=') {
        advance();
        if (peek() == '=') {
            advance();
            return Token(TokenType::STRICT_EQ, "===", line, col);
        }
        return Token(TokenType::EQ, "==", line, col);
    }
    return Token(TokenType::ASSIGN, "=", line, col);
}
```

### **Number Recognition**
```cpp
if (current() == '0' && (peek() == 'x' || peek() == 'X')) {
    // Hexadecimal number
    advance(); advance(); // Skip '0x'
    while (std::isxdigit(current())) {
        advance();
    }
}
```

### **Comment Detection**
```cpp
if (current() == '/' && peek() == '/') {
    // Single-line comment
    advance(); advance();
    skipComment();
}
```

## Integration Points

### **With advance() Function**
```cpp
char next = peek();
if (next == '=') {
    advance();  // Consume current character
    advance();  // Consume peeked character
}
```

### **With current() Function**
```cpp
char curr = current();
char next = peek();
if (curr == next && curr == '+') {
    // Found ++ operator
}
```

### **In Token Recognition**
```cpp
Token readOperator() {
    char c = current();
    char n1 = peek(1);
    char n2 = peek(2);
    
    // Complex operator recognition logic
    if (c == '>' && n1 == '>' && n2 == '=') {
        advance(); advance(); advance();
        return Token(TokenType::RSHIFT_ASSIGN, ">>=", line, col);
    }
    // ... other cases
}
```

## Error Handling and Edge Cases

### **End of Input Handling**
```cpp
char next = peek();
if (next == '\0') {
    // At end of source
    return Token(TokenType::EOF_TOKEN, "", line, col);
}
```

### **Large Offsets**
```cpp
char far = peek(1000);  // Likely returns '\0'
```

### **Negative Offsets**
```cpp
char prev = peek(-1);  // Look at previous character
if (prev == '\0') {
    // At start of source
}
```

## Performance Optimization

### **Inline Optimization**
```cpp
inline char Lexer::peek(int offset) const {
    size_t p = pos + offset;
    return p < src.size() ? src[p] : '\0';
}
```

### **Branch Prediction**
```cpp
// Most peek calls will be in bounds
return p < src.size() ? src[p] : '\0';
// Optimized for true condition
```

### **Common Case Optimization**
```cpp
// Special case for peek(1)
char peek1() const {
    return pos + 1 < src.size() ? src[pos + 1] : '\0';
}
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Infinite Loop Prevention**
```cpp
while (current() != '\0') {
    char next = peek();
    if (next == '\0') {
        break;  // End of input approaching
    }
    // Process character
    advance();
}
```

#### **Offset Validation**
```cpp
char peekSafe(int offset) const {
    if (offset < -100 || offset > 100) {
        return '\0';  // Reasonable offset limits
    }
    return peek(offset);
}
```

### **Debugging Support**
```cpp
char Lexer::peek(int offset) const {
    size_t p = pos + offset;
    char result = p < src.size() ? src[p] : '\0';
    
    if (getenv("DEBUG_LEXER")) {
        std::cout << "peek(" << offset << "): pos=" << pos 
                  << " result='" << result << "' (" 
                  << (int)result << ")\n";
    }
    
    return result;
}
```

## Extension Points

### **Multi-Peek Function**
```cpp
std::string peekMultiple(int count) const {
    std::string result;
    for (int i = 0; i < count; ++i) {
        char c = peek(i);
        if (c == '\0') break;
        result += c;
    }
    return result;
}
```

### **Pattern Matching**
```cpp
bool peekPattern(const std::string &pattern) const {
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (peek(i) != pattern[i]) {
            return false;
        }
    }
    return true;
}
```

### **Unicode Support**
```cpp
char32_t peekUnicode(int offset) const {
    // Handle UTF-8 multi-byte characters
    // Return Unicode codepoint instead of byte
}
```

## Advanced Usage Examples

### **Complex Operator Recognition**
```cpp
Token readComplexOperator() {
    if (current() == '>' && peek() == '>' && peek(1) == '>') {
        if (peek(2) == '=') {
            // >>>= operator
            advance(); advance(); advance(); advance();
            return Token(TokenType::RSHIFT_ASSIGN, ">>>=", line, col);
        } else {
            // >>> operator
            advance(); advance(); advance();
            return Token(TokenType::RSHIFT, ">>>", line, col);
        }
    }
    // ... other cases
}
```

### **String Interpolation Detection**
```cpp
if (current() == '$' && peek() == '{') {
    // Start of string interpolation
    advance(); advance();  // Skip ${
    // Read interpolated expression
}
```

### **Regex Literal Detection**
```cpp
if (current() == '/' && peek() != '/' && peek() != '*') {
    // Potential regex literal
    // Look ahead to confirm pattern
    bool isRegex = detectRegexLiteral();
    if (isRegex) {
        return readRegexLiteral();
    }
}
```

## Why This Design Works

### **Flexibility**
- **Variable Lookahead**: Can peek any number of characters
- **Non-Destructive**: Examines input without consuming it
- **Multiple Uses**: Works for operators, strings, comments, etc.
- **Context Awareness**: Enables context-sensitive parsing

### **Safety**
- **Bounds Checking**: Never accesses invalid memory
- **Consistent Return**: Always returns valid character
- **No State Modification**: Preserves lexer position
- **Exception Safe**: Never throws exceptions

### **Performance**
- **Constant Time**: O(1) direct array access
- **Minimal Overhead**: Simple arithmetic and bounds check
- **Cache Friendly**: Sequential memory access pattern
- **Inline Optimization**: Likely inlined by compiler

### **Usability**
- **Intuitive Interface**: Clear function name and parameters
- **Default Parameter**: Convenient default offset of 1
- **Predictable Behavior**: Consistent results across calls
- **Well-Documented**: Clear intent and behavior

The `peek()` function demonstrates excellent utility design, providing a flexible, safe, and efficient lookahead mechanism that enables sophisticated token recognition and parsing decisions while maintaining bounds safety and consistent behavior.
