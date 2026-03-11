# readNumber() Function - Numeric Literal Tokenization

## Overview
The `readNumber()` function is a comprehensive numeric literal parser that recognizes and tokenizes various number formats including decimal numbers, hexadecimal literals, floating-point numbers, and C-style type suffixes. It provides essential support for multiple programming language numeric conventions while maintaining accurate position tracking for error reporting.

## Complete Source Code

```cpp
Token Lexer::readNumber()
{
    int startLine = line, startCol = col;
    std::string num;
    bool hasDot = false;

    if (current() == '0' && (peek() == 'x' || peek() == 'X'))
    {
        num += advance();
        num += advance(); // 0x
        while (pos < src.size() && std::isxdigit(current()))
            num += advance();
    }
    else
    {
        while (pos < src.size() && (std::isdigit(current()) || current() == '.'))
        {
            if (current() == '.')
            {
                if (hasDot)
                    break;
                hasDot = true;
            }
            num += advance();
        }
        // Strip C integer/float suffixes: LL, ULL, LU, L, U, F, f (e.g. 1000000007LL, 3.14f)
        while (pos < src.size() && (current() == 'L' || current() == 'l' ||
                                    current() == 'U' || current() == 'u' ||
                                    current() == 'F' || current() == 'f'))
            advance(); // consume but don't add to num
    }
    return Token(TokenType::NUMBER, num, startLine, startCol);
}
```

## Line-by-Line Analysis

### **Function Signature and Initialization (Lines 1-5)**
```cpp
Token Lexer::readNumber()
{
    int startLine = line, startCol = col;
    std::string num;
    bool hasDot = false;
```

**Initialization Analysis:**
- **`Token` Return Type**: Returns a NUMBER token with the parsed numeric value
- **Position Tracking**: Records starting line and column for error reporting
- **String Accumulator**: `num` string builds the numeric literal text
- **Decimal Point Flag**: `hasDot` tracks whether a decimal point was encountered

### **Hexadecimal Number Detection (Lines 7-13)**
```cpp
if (current() == '0' && (peek() == 'x' || peek() == 'X'))
{
    num += advance();
    num += advance(); // 0x
    while (pos < src.size() && std::isxdigit(current()))
        num += advance();
}
```

**Hexadecimal Processing:**
- **Prefix Detection**: Checks for `0x` or `0X` prefix
- **Prefix Consumption**: Consumes both `0` and `x/X` characters
- **Digit Collection**: Collects hexadecimal digits (0-9, A-F, a-f)
- **Termination**: Stops at first non-hexadecimal character

**Hexadecimal Features:**
- **Standard Prefix**: Uses C-style `0x` prefix
- **Case Insensitive**: Accepts both `0x` and `0X`
- **Full Range**: Supports all 16 hexadecimal digits
- **Error Prevention**: Stops gracefully on invalid digits

### **Decimal and Floating-Point Processing (Lines 14-26)**
```cpp
else
{
    while (pos < src.size() && (std::isdigit(current()) || current() == '.'))
    {
        if (current() == '.')
        {
            if (hasDot)
                break;
            hasDot = true;
        }
        num += advance();
    }
```

**Decimal Number Logic:**
- **Digit Collection**: Collects decimal digits (0-9)
- **Decimal Point Handling**: Allows exactly one decimal point
- **Floating Point Support**: Recognizes floating-point numbers
- **Multiple Dots Prevention**: Breaks on second decimal point

**Floating-Point Features:**
- **Single Decimal Point**: Only one `.` allowed per number
- **Mixed Formats**: Supports both integers and floats
- **Graceful Termination**: Stops at invalid characters
- **Position Tracking**: Maintains accurate position throughout

### **C-Style Suffix Stripping (Lines 27-31)**
```cpp
// Strip C integer/float suffixes: LL, ULL, LU, L, U, F, f (e.g. 1000000007LL, 3.14f)
while (pos < src.size() && (current() == 'L' || current() == 'l' ||
                            current() == 'U' || current() == 'u' ||
                            current() == 'F' || current() == 'f'))
    advance(); // consume but don't add to num
```

**Suffix Handling:**
- **Integer Suffixes**: `L`, `LL`, `U`, `UL`, `ULL`
- **Float Suffixes**: `F`, `f`
- **Case Insensitive**: Accepts both uppercase and lowercase
- **Non-Adding**: Consumes suffixes but doesn't include in token value

**Supported Suffixes:**
- **`L`, `l`**: Long integer
- **`LL`, `ll`**: Long long integer  
- **`U`, `u`**: Unsigned integer
- **`UL`, `ul`**: Unsigned long
- **`ULL`, `ull`**: Unsigned long long
- **`F`, `f`**: Float

### **Token Creation (Line 33)**
```cpp
return Token(TokenType::NUMBER, num, startLine, startCol);
```

**Token Construction:**
- **Token Type**: `NUMBER` token type
- **Token Value**: The parsed numeric string (without suffixes)
- **Position Information**: Starting line and column for error reporting
- **Clean Return**: Properly formed token for parser consumption

## Design Patterns and Architecture

### **State Machine Pattern**
```cpp
Token readNumber() {
    if (isHexPrefix()) {
        readHexDigits();
    } else {
        readDecimalDigits();
        readOptionalSuffixes();
    }
    return createNumberToken();
}
```
- **State-Based Parsing**: Different logic paths for different number types
- **Clear Transitions**: Obvious decision points between number formats
- **Extensible**: Easy to add new number formats
- **Maintainable**: Each format handled in separate section

### **Consumption Pattern**
```cpp
while (isValidDigit(current())) {
    num += advance();  // Consume and accumulate
}
```
- **Character Consumption**: Advances while consuming valid characters
- **String Building**: Accumulates characters into token value
- **Termination Condition**: Stops at first invalid character
- **Position Tracking**: Automatic position tracking through advance()

### **Validation Pattern**
```cpp
if (current() == '.') {
    if (hasDot) break;  // Validation: prevent multiple dots
    hasDot = true;      // State update
}
```
- **Inline Validation**: Validates during parsing
- **State Tracking**: Tracks parsing state (hasDot)
- **Early Termination**: Breaks on invalid input
- **Error Prevention**: Prevents malformed numbers

## Technical Considerations

### **Number Format Support**
```cpp
// Supported formats:
123        // Decimal integer
123.45     // Floating point
0xFF       // Hexadecimal
123LL      // Long long with suffix
3.14f      // Float with suffix
```

**Format Features:**
- **Multiple Radix**: Decimal and hexadecimal support
- **Type Suffixes**: C-style type annotations
- **Floating Point**: Full floating-point support
- **Mixed Types**: Integer and float in single parser

### **Character Classification**
```cpp
std::isxdigit(current())  // Hexadecimal digits: 0-9, A-F, a-f
std::isdigit(current())  // Decimal digits: 0-9
```

**Classification Functions:**
- **Standard Library**: Uses `<cctype>` functions
- **Locale Independent**: Character classification is locale-independent
- **Efficient**: Optimized implementations
- **Portable**: Works across all platforms

### **Performance Characteristics**
```cpp
// Time complexity: O(n) where n = number of characters in numeric literal
// Space complexity: O(n) for the numeric string
```
- **Linear Processing**: Each character examined exactly once
- **String Building**: Accumulates characters in string
- **Early Termination**: Stops at first invalid character
- **Memory Efficient**: Only stores the numeric portion

## Usage Patterns

### **Basic Number Recognition**
```cpp
Token readToken() {
    if (std::isdigit(current())) {
        return readNumber();
    }
    // ... other token types
}
```

### **Integration with Tokenizer**
```cpp
std::vector<Token> Lexer::tokenize() {
    while (pos < src.size()) {
        skipWhitespace();
        char c = current();
        
        if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        }
        // ... other token types
    }
}
```

### **Error Reporting Integration**
```cpp
Token readNumber() {
    int startLine = line, startCol = col;
    // ... parsing logic
    
    if (num.empty()) {
        throw QuantumError("LexError", "Invalid numeric literal", startLine);
    }
    
    return Token(TokenType::NUMBER, num, startLine, startCol);
}
```

## Error Handling and Edge Cases

### **Invalid Hexadecimal Numbers**
```cpp
// Input: "0xGHI"
// Process: Consumes "0x", stops at 'G' (invalid hex digit)
// Result: Token with value "0x"
```

### **Multiple Decimal Points**
```cpp
// Input: "123.45.67"
// Process: Consumes "123.45", stops at second '.'
// Result: Token with value "123.45"
```

### **Numbers at End of Input**
```cpp
// Input: "42" at end of file
// Process: Consumes "42", reaches EOF
// Result: Token with value "42"
```

### **Empty Numbers**
```cpp
// Input: "." (not a number)
// Process: Not called (doesn't start with digit)
// Result: Handled by other token type logic
```

## Performance Optimization

### **Character Classification Optimization**
```cpp
// Fast digit checking
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isHexDigit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}
```

### **String Building Optimization**
```cpp
// Reserve space for common number sizes
num.reserve(16);  // Most numbers fit in 16 characters
while (condition) {
    num += advance();
}
```

### **Branch Prediction Optimization**
```cpp
// Most numbers are decimal, not hexadecimal
if (unlikely(current() == '0' && (peek() == 'x' || peek() == 'X'))) {
    // Unlikely path - hexadecimal
    readHexNumber();
} else {
    // Likely path - decimal
    readDecimalNumber();
}
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Position Tracking**
```cpp
// Correct: Record start position before parsing
int startLine = line, startCol = col;

// Wrong: Recording after consuming characters
int startLine = line, startCol = col;  // Now points to end, not start
```

#### **Suffix Handling**
```cpp
// Correct: Consume but don't add suffixes
while (isSuffix(current())) {
    advance();  // Don't add to num
}

// Wrong: Adding suffixes to token value
num += advance();  // Includes suffix in token value
```

### **Debugging Support**
```cpp
Token Lexer::readNumber() {
    int startLine = line, startCol = col;
    std::string num;
    bool hasDot = false;
    
    if (getenv("DEBUG_NUMBERS")) {
        std::cout << "Reading number at " << startLine << ":" << startCol << "\n";
    }
    
    // ... parsing logic
    
    if (getenv("DEBUG_NUMBERS")) {
        std::cout << "Parsed number: '" << num << "'\n";
    }
    
    return Token(TokenType::NUMBER, num, startLine, startCol);
}
```

### **Validation Testing**
```cpp
void testNumberParsing() {
    struct TestCase {
        std::string input;
        std::string expected;
    };
    
    std::vector<TestCase> tests = {
        {"123", "123"},
        {"123.45", "123.45"},
        {"0xFF", "0xFF"},
        {"123LL", "123"},
        {"3.14f", "3.14"},
        {"123.45.67", "123.45"}  // Multiple dots
    };
    
    for (const auto &test : tests) {
        Lexer lexer(test.input);
        Token token = lexer.readNumber();
        assert(token.value == test.expected);
    }
}
```

## Extension Points

### **Binary Number Support**
```cpp
if (current() == '0' && (peek() == 'b' || peek() == 'B')) {
    num += advance();
    num += advance(); // 0b
    while (pos < src.size() && (current() == '0' || current() == '1'))
        num += advance();
}
```

### **Octal Number Support**
```cpp
if (current() == '0' && std::isdigit(peek()) && peek() <= '7') {
    num += advance();
    while (pos < src.size() && current() >= '0' && current() <= '7')
        num += advance();
}
```

### **Scientific Notation**
```cpp
// Add to decimal processing
if (current() == 'e' || current() == 'E') {
    num += advance();
    if (current() == '+' || current() == '-') {
        num += advance();
    }
    while (std::isdigit(current())) {
        num += advance();
    }
}
```

### **Unicode Numeric Support**
```cpp
bool isUnicodeDigit(char32_t c) {
    // Support for Unicode numeric characters
    return (c >= '0' && c <= '9') ||  // ASCII digits
           (c >= 0x0660 && c <= 0x0669) ||  // Arabic-Indic digits
           (c >= 0x06F0 && c <= 0x06F9);   // Extended Arabic-Indic digits
}
```

## Advanced Usage Examples

### **Number with Base Detection**
```cpp
struct NumberInfo {
    std::string value;
    int base;
    bool isFloat;
    std::string suffix;
};

NumberInfo readNumberInfo() {
    NumberInfo info;
    
    if (current() == '0' && (peek() == 'x' || peek() == 'X')) {
        info.base = 16;
        info.value = readHexNumber();
    } else if (current() == '0' && (peek() == 'b' || peek() == 'B')) {
        info.base = 2;
        info.value = readBinaryNumber();
    } else {
        info.base = 10;
        info.value = readDecimalNumber();
        info.isFloat = info.value.find('.') != std::string::npos;
    }
    
    info.suffix = readSuffix();
    return info;
}
```

### **Precise Number Validation**
```cpp
class NumberValidator {
public:
    bool isValidNumber(const std::string &input) {
        Lexer lexer(input);
        try {
            Token token = lexer.readNumber();
            return token.type == TokenType::NUMBER;
        } catch (...) {
            return false;
        }
    }
    
    std::string normalizeNumber(const std::string &input) {
        Lexer lexer(input);
        Token token = lexer.readNumber();
        return token.value;
    }
};
```

## Why This Design Works

### **Comprehensiveness**
- **Multiple Formats**: Supports decimal, hexadecimal, floating-point
- **C Compatibility**: Handles C-style type suffixes
- **Language Agnostic**: Works with multiple programming languages
- **Extensible**: Easy to add new number formats

### **Robustness**
- **Error Prevention**: Prevents malformed numbers (multiple dots)
- **Graceful Termination**: Stops at invalid characters
- **Bounds Safe**: Never accesses past end of source
- **Position Accurate**: Maintains accurate position tracking

### **Performance**
- **Linear Processing**: Each character examined once
- **Early Termination**: Stops at first invalid character
- **Minimal Overhead**: Simple character classification
- **Memory Efficient**: Only stores numeric portion

### **Maintainability**
- **Clear Structure**: Separate sections for different number types
- **Self-Documenting**: Function and variable names are clear
- **Testable**: Easy to unit test with various inputs
- **Extensible**: Simple to add new number formats

The `readNumber()` function demonstrates excellent lexical analyzer design, providing comprehensive numeric literal parsing with support for multiple programming language conventions while maintaining efficiency, accuracy, and extensibility for future enhancements.
