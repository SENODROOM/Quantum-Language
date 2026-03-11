# readString() Function - String Literal Tokenization

## Overview
The `readString()` function processes string literals enclosed in quotes, handling escape sequences, unterminated string detection, and proper position tracking. It supports both single and double quoted strings with comprehensive escape sequence processing, providing essential string literal support for the Quantum Language lexer.

## Complete Source Code

```cpp
Token Lexer::readString(char quote)
{
    int startLine = line, startCol = col;
    advance(); // skip opening quote
    std::string str;
    while (pos < src.size() && current() != quote)
    {
        if (current() == '\\')
        {
            advance();
            switch (current())
            {
            case 'n':
                str += '\n';
                break;
            case 't':
                str += '\t';
                break;
            case 'r':
                str += '\r';
                break;
            case '\\':
                str += '\\';
                break;
            case '\'':
                str += '\'';
                break;
            case '"':
                str += '"';
                break;
            case '0':
                str += '\0';
                break;
            default:
                str += current();
            }
            advance();
        }
        else
        {
            str += advance();
        }
    }
    if (pos >= src.size())
        throw QuantumError("LexError", "Unterminated string literal", startLine);
    advance(); // skip closing quote
    return Token(TokenType::STRING, str, startLine, startCol);
}
```

## Line-by-Line Analysis

### **Function Signature and Initialization (Lines 1-4)**
```cpp
Token Lexer::readString(char quote)
{
    int startLine = line, startCol = col;
    advance(); // skip opening quote
    std::string str;
```

**Initialization Analysis:**
- **`Token` Return Type**: Returns a STRING token with processed content
- **`char quote` Parameter**: The quote character used (`'` or `"`)
- **Position Tracking**: Records starting position for error reporting
- **Quote Consumption**: Advances past opening quote to start string content
- **String Builder**: `str` accumulates the processed string content

### **Main String Processing Loop (Lines 6-35)**
```cpp
while (pos < src.size() && current() != quote)
{
    if (current() == '\\')
    {
        // Escape sequence handling
    }
    else
    {
        str += advance();
    }
}
```

**Loop Condition Analysis:**
- **`pos < src.size()`**: Bounds checking to prevent accessing past end
- **`current() != quote`**: Continues until matching closing quote found
- **Inclusive Processing**: Processes all characters between quotes
- **Termination**: Stops at closing quote or end of source

### **Escape Sequence Handling (Lines 8-33)**
```cpp
if (current() == '\\')
{
    advance();
    switch (current())
    {
    case 'n':
        str += '\n';
        break;
    case 't':
        str += '\t';
        break;
    case 'r':
        str += '\r';
        break;
    case '\\':
        str += '\\';
        break;
    case '\'':
        str += '\'';
        break;
    case '"':
        str += '"';
        break;
    case '0':
        str += '\0';
        break;
    default:
        str += current();
    }
    advance();
}
```

**Escape Sequence Processing:**
- **Backslash Detection**: Identifies escape sequence start
- **Character Advancement**: Consumes backslash to get escape code
- **Switch Statement**: Maps escape codes to actual characters
- **Default Handling**: Unknown escape codes passed through unchanged

**Supported Escape Sequences:**
- **`\n`**: Newline character (0x0A)
- **`\t`**: Horizontal tab character (0x09)
- **`\r`**: Carriage return character (0x0D)
- **`\\`**: Backslash character (0x5C)
- **`\'`**: Single quote character (0x27)
- **`"`**: Double quote character (0x22)
- **`\0`**: Null character (0x00)
- **Unknown**: Pass through unchanged (e.g., `\x`, `\y`)

### **Regular Character Handling (Lines 34-36)**
```cpp
else
{
    str += advance();
}
```

**Regular Character Logic:**
- **Direct Addition**: Non-escaped characters added directly to string
- **Position Advancement**: advance() handles position tracking
- **Efficient Processing**: No additional processing needed
- **Content Preservation**: Original character preserved exactly

### **Error Detection (Lines 38-40)**
```cpp
if (pos >= src.size())
    throw QuantumError("LexError", "Unterminated string literal", startLine);
```

**Error Handling:**
- **End Detection**: Checks if reached end of source without closing quote
- **Error Throwing**: Throws QuantumError with descriptive message
- **Position Information**: Uses starting position for error location
- **Clear Message**: "Unterminated string literal" is self-explanatory

### **Closing Quote Consumption (Line 42)**
```cpp
advance(); // skip closing quote
```

**Quote Handling:**
- **Consumption**: Advances past closing quote
- **Position Update**: Updates lexer position for next token
- **Loop Completion**: String processing is complete
- **Consistency**: Matches opening quote consumption

### **Token Creation (Line 44)**
```cpp
return Token(TokenType::STRING, str, startLine, startCol);
```

**Token Construction:**
- **Token Type**: `STRING` token type
- **Processed Content**: String with escape sequences processed
- **Position Information**: Starting position for error reporting
- **Clean Return**: Properly formed token for parser consumption

## Design Patterns and Architecture

### **State Machine Pattern**
```cpp
Token readString(char quote) {
    advance();  // Skip opening quote
    
    while (current() != quote) {
        if (current() == '\\') {
            handleEscapeSequence();
        } else {
            handleRegularCharacter();
        }
    }
    
    advance();  // Skip closing quote
    return createStringToken();
}
```
- **State-Based Processing**: Different handling for escaped vs regular characters
- **Termination Condition**: Clear stop condition (closing quote)
- **Error Handling**: Detects unterminated strings
- **Clean Structure**: Logical flow from start to finish

### **Escape Sequence Pattern**
```cpp
if (current() == '\\') {
    advance();  // Skip backslash
    char code = current();
    char result = translateEscapeCode(code);
    advance();  // Skip escape code
    return result;
}
```
- **Two-Character Sequence**: Backslash + escape code
- **Translation Function**: Maps codes to actual characters
- **Consumption**: Consumes both characters of sequence
- **Default Handling**: Unknown codes passed through

### **Error Detection Pattern**
```cpp
if (pos >= src.size()) {
    throw QuantumError("LexError", "Unterminated string literal", startLine);
}
```
- **Pre-Validation**: Check for error conditions before continuing
- **Exception Throwing**: Uses language-specific error handling
- **Context Preservation**: Maintains error location information
- **Clear Messaging**: Descriptive error messages

## Technical Considerations

### **Quote Type Support**
```cpp
// Single quotes
Token token1 = readString('\'');  // 'Hello'

// Double quotes  
Token token2 = readString('"');   // "World"
```

**Quote Features:**
- **Flexible**: Supports both single and double quotes
- **Consistent Processing**: Same logic for both quote types
- **Mixed Usage**: Can use both types in same source file
- **Language Compatibility**: Matches common programming language conventions

### **Escape Sequence Processing**
```cpp
// Standard C-style escape sequences
"\n"  // Newline
"\t"  // Tab
"\\"  // Quote
"\\\\" // Backslash
```

**Escape Features:**
- **C Compatibility**: Standard C-style escape sequences
- **Complete Coverage**: All common escape codes supported
- **Null Character**: Supports `\0` for embedded nulls
- **Fallback**: Unknown escape codes passed through

### **Position Tracking Accuracy**
```cpp
int startLine = line, startCol = col;  // Record start
advance();  // Skip opening quote
// ... string processing
return Token(TokenType::STRING, str, startLine, startCol);
```

**Position Features:**
- **Start Position**: Records where string begins
- **Error Reporting**: Accurate error location for unterminated strings
- **User Experience**: Line/column matches editor position
- **Consistency**: Same position tracking as other tokens

## Usage Patterns

### **Basic String Recognition**
```cpp
Token readToken() {
    char c = current();
    if (c == '\'' || c == '"') {
        return readString(c);
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
        
        if (c == '\'' || c == '"') {
            tokens.push_back(readString(c));
        }
        // ... other token types
    }
}
```

### **Error Handling Integration**
```cpp
try {
    Token token = readString(current());
    tokens.push_back(token);
} catch (const QuantumError &e) {
    // Handle unterminated string error
    std::cerr << "Error: " << e.what() << " at line " << e.line << "\n";
    // Decide whether to continue or abort
}
```

## Error Handling and Edge Cases

### **Unterminated Strings**
```cpp
// Input: "Hello, World
// Error: Unterminated string literal at line 1
```

### **Empty Strings**
```cpp
// Input: ""
// Result: Token with value "" (empty string)
```

### **Strings with Escapes**
```cpp
// Input: "Hello\nWorld"
// Result: Token with value "Hello\nWorld" (contains actual newline)
```

### **Unknown Escape Sequences**
```cpp
// Input: "Hello\xWorld"
// Result: Token with value "HelloxWorld" (unknown escape passed through)
```

### **Embedded Quotes**
```cpp
// Input: "He said \"Hello\""
// Result: Token with value 'He said "Hello"'
```

## Performance Optimization

### **String Building Optimization**
```cpp
// Reserve space for common string sizes
str.reserve(64);  // Most strings fit in 64 characters
while (condition) {
    str += processed_char;
}
```

### **Escape Sequence Optimization**
```cpp
// Fast escape code lookup
static const std::unordered_map<char, char> escapeMap = {
    {'n', '\n'}, {'t', '\t'}, {'r', '\r'}, {'\\', '\\'},
    {'\'', '\''}, {'"', '"'}, {'0', '\0'}
};

char translateEscape(char code) {
    auto it = escapeMap.find(code);
    return (it != escapeMap.end()) ? it->second : code;
}
```

### **Branch Prediction Optimization**
```cpp
while (pos < src.size() && current() != quote) {
    char c = current();
    if (unlikely(c == '\\')) {
        // Uncommon path - escape sequence
        handleEscape();
    } else {
        // Common path - regular character
        str += c;
        advance();
    }
}
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Position Tracking**
```cpp
// Correct: Record start before consuming opening quote
int startLine = line, startCol = col;
advance();  // Skip opening quote

// Wrong: Record after consuming opening quote
advance();  // Now points to string content, not start
int startLine = line, startCol = col;
```

#### **Escape Sequence Handling**
```cpp
// Correct: Consume both backslash and escape code
if (current() == '\\') {
    advance();  // Skip backslash
    char code = current();
    advance();  // Skip escape code
    // Process code...
}

// Wrong: Only consume backslash
if (current() == '\\') {
    advance();  // Skip backslash only
    // Escape code becomes next character in string
}
```

### **Debugging Support**
```cpp
Token Lexer::readString(char quote) {
    int startLine = line, startCol = col;
    advance();
    std::string str;
    
    if (getenv("DEBUG_STRINGS")) {
        std::cout << "Reading string at " << startLine << ":" << startCol 
                  << " with quote '" << quote << "'\n";
    }
    
    while (pos < src.size() && current() != quote) {
        if (current() == '\\') {
            if (getenv("DEBUG_STRINGS")) {
                std::cout << "  Escape sequence: \\" << peek() << "\"\n";
            }
            // ... escape processing
        } else {
            str += advance();
        }
    }
    
    if (getenv("DEBUG_STRINGS")) {
        std::cout << "  Parsed string: \"" << str << "\"\n";
    }
    
    // ... rest of function
}
```

### **Validation Testing**
```cpp
void testStringParsing() {
    struct TestCase {
        std::string input;
        std::string expected;
        bool shouldSucceed;
    };
    
    std::vector<TestCase> tests = {
        {"\"Hello\"", "Hello", true},
        {"\"Hello\\nWorld\"", "Hello\nWorld", true},
        {"\"Hello\\xWorld\"", "HelloxWorld", true},
        {"\"Unterminated", "", false},
        {"\"\"", "", true}
    };
    
    for (const auto &test : tests) {
        Lexer lexer(test.input);
        try {
            Token token = lexer.readString('"');
            assert(test.shouldSucceed);
            assert(token.value == test.expected);
        } catch (...) {
            assert(!test.shouldSucceed);
        }
    }
}
```

## Extension Points

### **Unicode Escape Sequences**
```cpp
case 'u':
    // Unicode escape sequence \uXXXX
    if (pos + 4 < src.size()) {
        uint16_t code = 0;
        for (int i = 0; i < 4; ++i) {
            char c = advance();
            code = (code << 4) + hexToNibble(c);
        }
        str += unicodeToUtf8(code);
    }
    break;
```

### **Raw String Support**
```cpp
Token readRawString(char quote) {
    advance();  // Skip opening quote
    std::string raw;
    while (pos < src.size() && current() != quote) {
        raw += advance();  // No escape processing
    }
    advance();  // Skip closing quote
    return Token(TokenType::STRING, raw, startLine, startCol);
}
```

### **Multi-Line Strings**
```cpp
Token readMultiLineString() {
    advance();  // Skip opening triple quote
    std::string content;
    while (pos < src.size()) {
        if (current() == '"' && peek() == '"' && peek(2) == '"') {
            advance(); advance(); advance();  // Skip closing triple quotes
            break;
        }
        content += advance();
    }
    return Token(TokenType::STRING, content, startLine, startCol);
}
```

### **String Interpolation**
```cpp
if (current() == '$' && peek() == '{') {
    // String interpolation: ${expression}
    advance(); advance();  // Skip ${
    std::string expr = readInterpolationExpression();
    str += evaluateExpression(expr);
}
```

## Advanced Usage Examples

### **String Validation**
```cpp
class StringValidator {
public:
    bool isValidString(const std::string &input) {
        try {
            Lexer lexer(input);
            Token token = lexer.readString(input[0]);
            return token.type == TokenType::STRING;
        } catch (...) {
            return false;
        }
    }
    
    std::string processEscapes(const std::string &input) {
        Lexer lexer("\"" + input + "\"");
        Token token = lexer.readString('"');
        return token.value;
    }
};
```

### **String Statistics**
```cpp
struct StringStats {
    int totalLength = 0;
    int processedLength = 0;
    int escapeCount = 0;
    std::set<char> escapeTypes;
};

StringStats analyzeString(const std::string &source) {
    Lexer lexer(source);
    StringStats stats;
    
    // Custom string reading with statistics
    // ... implementation
    
    return stats;
}
```

## Why This Design Works

### **Comprehensiveness**
- **Complete Escape Support**: All standard C-style escape sequences
- **Quote Flexibility**: Supports both single and double quotes
- **Error Detection**: Proper handling of unterminated strings
- **Position Accuracy**: Precise error location reporting

### **Robustness**
- **Bounds Checking**: Never accesses past end of source
- **Graceful Handling**: Unknown escape codes passed through
- **Error Recovery**: Clear error messages for malformed strings
- **Consistent Behavior**: Predictable processing across all inputs

### **Performance**
- **Linear Processing**: Each character examined exactly once
- **Efficient Building**: String accumulation with minimal overhead
- **Early Termination**: Stops at closing quote or end of input
- **Memory Efficient**: Only stores processed string content

### **Maintainability**
- **Clear Structure**: Logical separation of regular vs escaped characters
- **Self-Documenting**: Function and variable names are clear
- **Testable**: Easy to unit test with various string inputs
- **Extensible**: Simple to add new escape sequences or features

The `readString()` function demonstrates excellent lexical analyzer design, providing comprehensive string literal processing with full escape sequence support, robust error handling, and accurate position tracking essential for quality error reporting and parser integration.
