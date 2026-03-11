# advance() Function - Position Advancement and Tracking

## Overview
The `advance()` function is the core position management function of the lexer. It moves the lexer's position forward by one character while automatically tracking line and column numbers for accurate error reporting and token location information. This function is fundamental to the lexical analysis process, enabling sequential processing of source code.

## Complete Source Code

```cpp
char Lexer::advance()
{
    char c = src[pos++];
    if (c == '\n')
    {
        line++;
        col = 1;
    }
    else
    {
        col++;
    }
    return c;
}
```

## Line-by-Line Analysis

### **Function Signature (Line 1)**
```cpp
char Lexer::advance()
```
- **`char`**: Returns the character being advanced past
- **`Lexer::advance()`**: Member function of Lexer class
- **No Parameters**: Advances by exactly one character
- **State Modification**: Updates lexer position, line, and column

### **Character Retrieval and Position Update (Line 3)**
```cpp
char c = src[pos++];
```
- **`char c`**: Stores the character being advanced past
- **`src[pos++]`**: Post-increment operator - returns current character then increments position
- **Atomic Operation**: Character retrieval and position advance happen together
- **Efficient**: Single operation for both retrieval and advancement

### **Newline Handling (Lines 5-9)**
```cpp
if (c == '\n')
{
    line++;
    col = 1;
}
```

**Newline Processing:**
- **Detection**: Checks if character is newline (`\n`)
- **Line Increment**: Increments line number by 1
- **Column Reset**: Resets column to 1 (start of new line)
- **1-Based Indexing**: User-friendly line/column numbering

### **Regular Character Handling (Lines 10-13)**
```cpp
else
{
    col++;
}
```

**Regular Character Processing:**
- **Non-Newline**: Handles all characters except `\n`
- **Column Increment**: Increments column by 1
- **Continuation**: Maintains current line number
- **Consistent Tracking**: Maintains accurate position information

### **Character Return (Line 15)**
```cpp
return c;
```
- **Return Value**: Returns the character that was advanced past
- **Useful Information**: Caller can examine the character just processed
- **Consistency**: Matches behavior of standard iterator operations
- **Convenience**: Eliminates need for separate current() call after advance()

## Design Patterns and Architecture

### **Iterator Pattern**
```cpp
char c = advance();  // Get current and move to next
// Similar to iterator: *it++
```
- **Sequential Access**: Moves through source code sequentially
- **Current Element**: Returns current element before advancement
- **Forward Movement**: Only moves forward, never backward
- **Stateful**: Maintains internal position state

### **State Management Pattern**
```cpp
class Lexer {
    size_t pos;  // Character position
    int line;    // Line number
    int col;     // Column number
    
public:
    char advance() {
        // Update all state variables consistently
    }
};
```
- **Consistent State**: All position variables updated together
- **Atomic Updates**: Position changes are atomic within function
- **Correlated Data**: Line and column correlate with character position
- **Maintenance**: Single point of position state changes

### **Position Tracking Pattern**
```cpp
char advance() {
    char c = src[pos++];
    updatePosition(c);  // Abstract position update
    return c;
}
```
- **Separation of Concerns**: Character retrieval from position tracking
- **Extensible**: Easy to add new position tracking features
- **Testable**: Position tracking logic can be tested independently
- **Maintainable**: Clear separation of responsibilities

## Technical Considerations

### **Position Tracking Accuracy**
```cpp
// Position coordinate systems
size_t pos;   // 0-based character index
int line;     // 1-based line number  
int col;      // 1-based column number
```
- **Dual Systems**: Internal (0-based) and external (1-based) coordinates
- **Consistency**: All coordinate systems stay synchronized
- **User Experience**: 1-based coordinates match editor conventions
- **Implementation**: 0-based indexing simplifies internal operations

### **Newline Handling**
```cpp
// Different newline conventions
if (c == '\n') {  // Unix/Linux style
    line++;
    col = 1;
}
// Could be extended for Windows \r\n
```
- **Cross-Platform**: Handles Unix-style newlines
- **Extensible**: Could be extended for Windows `\r\n`
- **Standard**: `\n` is standard across most systems
- **Simplicity**: Single character newline detection

### **Performance Characteristics**
```cpp
// Time complexity: O(1) - constant time operations
// Space complexity: O(1) - no additional memory allocation
```
- **Constant Time**: Simple arithmetic and array access
- **No Memory Allocation**: Uses existing member variables
- **Cache Friendly**: Sequential memory access pattern
- **Inline Optimization**: Likely inlined by compiler

## Usage Patterns

### **Basic Token Reading**
```cpp
char c = advance();
if (std::isdigit(c)) {
    // Start reading number token
}
```

### **String Literal Processing**
```cpp
if (advance() == '"') {
    std::string content;
    while (advance() != '"') {
        content += current();
    }
}
```

### **Comment Skipping**
```cpp
if (advance() == '/' && advance() == '/') {
    while (advance() != '\n') {
        // Skip comment content
    }
}
```

### **Multi-Character Operators**
```cpp
char first = advance();
char second = peek();
if (first == '+' && second == '+') {
    advance();  // Consume second '+'
    return Token(TokenType::PLUS_PLUS, "++", line, col);
}
```

## Integration Points

### **With current() Function**
```cpp
char c = advance();  // Get current and advance
// Now current() returns next character
```

### **With peek() Function**
```cpp
char current = advance();  // Advance and get current
char next = peek();        // Look ahead to next character
```

### **In Token Recognition Loops**
```cpp
while (pos < src.size()) {
    char c = advance();
    switch (c) {
        case '(': processOpeningParen(); break;
        case ')': processClosingParen(); break;
        // ... other cases
    }
}
```

## Error Handling and Edge Cases

### **End of Input Handling**
```cpp
char c = advance();
if (c == '\0') {
    // Reached end of source
    return TokenType::EOF_TOKEN;
}
```

### **Position Validation**
```cpp
char advance() {
    if (pos >= src.size()) {
        return '\0';  // Already at end
    }
    char c = src[pos++];
    // ... position tracking
    return c;
}
```

### **Large Files**
```cpp
// Position overflow protection
if (pos > std::numeric_limits<size_t>::max() - 1) {
    throw QuantumError("LexError", "Source file too large", line);
}
```

## Performance Optimization

### **Inline Optimization**
```cpp
inline char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') { line++; col = 1; } else { col++; }
    return c;
}
```

### **Branch Prediction**
```cpp
// Most characters are not newlines
if (c == '\n') {
    // Uncommon case
} else {
    // Common case - optimized by branch predictor
    col++;
}
```

### **Loop Unrolling**
```cpp
// For high-performance tokenization
char c1 = advance();
char c2 = advance();
char c3 = advance();
char c4 = advance();
// Process multiple characters at once
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Position Tracking Errors**
```cpp
// Wrong: forgetting to update position
char c = src[pos++];  // Position updated
// Missing line/col update
```

#### **Off-by-One Errors**
```cpp
// Correct: advance() returns current character, then moves
char c = advance();  // c is current, pos is now next

// Wrong: assuming advance() returns next character
char c = advance();  // c is NOT next character
```

### **Debugging Support**
```cpp
char Lexer::advance() {
    char c = src[pos++];
    
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    
    if (getenv("DEBUG_LEXER")) {
        std::cout << "advance(): '" << c << "' (" << (int)c 
                  << ") pos=" << pos << " line=" << line 
                  << " col=" << col << "\n";
    }
    
    return c;
}
```

### **Position Validation**
```cpp
void validatePosition() const {
    assert(pos <= src.size());
    assert(line >= 1);
    assert(col >= 1);
    // Additional invariants
}
```

## Extension Points

### **Tab Handling**
```cpp
char advance() {
    char c = src[pos++];
    if (c == '\n') {
        line++;
        col = 1;
    } else if (c == '\t') {
        col = ((col - 1 + 8) / 8) * 8 + 1;  // Tab to 8-space boundary
    } else {
        col++;
    }
    return c;
}
```

### **Unicode Support**
```cpp
char32_t advanceUnicode() {
    // Handle UTF-8 multi-byte characters
    // Return Unicode codepoint instead of byte
}
```

### **Position History**
```cpp
struct Position {
    size_t pos;
    int line;
    int col;
};

std::vector<Position> positionHistory;

char advance() {
    Position oldPos{pos, line, col};
    positionHistory.push_back(oldPos);
    // ... normal advance logic
}
```

## Advanced Usage Examples

### **Backtracking Support**
```cpp
class Lexer {
    std::vector<size_t> positionStack;
    std::vector<int> lineStack;
    std::vector<int> colStack;
    
    void pushPosition() {
        positionStack.push_back(pos);
        lineStack.push_back(line);
        colStack.push_back(col);
    }
    
    void popPosition() {
        pos = positionStack.back();
        line = lineStack.back();
        col = colStack.back();
        positionStack.pop_back();
        lineStack.pop_back();
        colStack.pop_back();
    }
};
```

### **Position Mapping**
```cpp
struct PositionMap {
    std::vector<size_t> lineStarts;
    
    void buildMap(const std::string &source) {
        lineStarts.push_back(0);  // Line 1 starts at 0
        for (size_t i = 0; i < source.size(); ++i) {
            if (source[i] == '\n') {
                lineStarts.push_back(i + 1);
            }
        }
    }
    
    std::pair<int, int> getPosition(size_t pos) {
        // Binary search to find line
        // Calculate column within line
    }
};
```

## Why This Design Works

### **Reliability**
- **Atomic Updates**: Position state changes are atomic
- **Consistent Tracking**: All position variables stay synchronized
- **Bounds Safe**: Handles end-of-input gracefully
- **Predictable**: Same behavior for all input conditions

### **Performance**
- **Constant Time**: O(1) operations with minimal overhead
- **Sequential Access**: Optimized for forward-only processing
- **Cache Friendly**: Linear memory access pattern
- **Inline Optimization**: Likely inlined by compiler

### **Usability**
- **Intuitive Interface**: Clear function name and behavior
- **Convenient Return**: Returns character just advanced past
- **Automatic Tracking**: No manual position management needed
- **Error Reporting**: Provides accurate position information

### **Maintainability**
- **Single Responsibility**: Only handles position advancement
- **Clear Logic**: Simple, straightforward implementation
- **Extensible**: Easy to add new position tracking features
- **Testable**: Simple to unit test with predictable results

The `advance()` function demonstrates excellent utility design, providing a reliable, efficient, and user-friendly mechanism for sequential source code processing with accurate position tracking essential for quality error reporting and token location information.
