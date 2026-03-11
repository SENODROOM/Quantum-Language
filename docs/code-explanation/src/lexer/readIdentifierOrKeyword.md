# readIdentifierOrKeyword() Function - Identifier and Keyword Recognition

## Overview
The `readIdentifierOrKeyword()` function processes alphanumeric sequences to determine whether they represent identifiers or language keywords. It handles raw strings, f-strings, macro expansion, and keyword lookup using a hash table, providing essential lexical analysis for variable names, function names, and language constructs.

## Complete Source Code

```cpp
Token Lexer::readIdentifierOrKeyword()
{
    int startLine = line, startCol = col;
    std::string id;
    while (pos < src.size() && (std::isalnum(current()) || current() == '_'))
        id += advance();

    // Raw string prefix: r"..." or r'...' — literal string with no escape sequences
    if ((id == "r" || id == "R") && pos < src.size() && (current() == '"' || current() == '\''))
    {
        char quote = current();
        int strStartLine = line, strStartCol = col;
        advance(); // skip opening quote
        std::string raw;
        while (pos < src.size() && current() != quote)
        {
            raw += advance();
        }
        if (pos < src.size())
            advance(); // skip closing quote
        return Token(TokenType::STRING, raw, strStartLine, strStartCol);
    }

    // f-string prefix: f"..." or f'...'  — treat like a backtick template literal
    if ((id == "f" || id == "F") && pos < src.size() && (current() == '"' || current() == '\''))
    {
        char quote = current();
        advance(); // skip opening quote
        // Convert {expr} → ${expr} then re-lex as template
        std::string raw;
        while (pos < src.size() && current() != quote)
        {
            if (current() == '{')
            {
                advance(); // skip {
                std::string exprPart;
                std::string fmtPart;
                int depth = 1;
                bool inFormat = false;
                int parenDepth = 0;
                bool inSingleQ = false;
                bool inDoubleQ = false;
                while (pos < src.size() && depth > 0)
                {
                    char ch = current();
                    // Track string literals inside the expression to avoid false colon matches
                    if (!inDoubleQ && ch == '\'' && !inFormat)
                        inSingleQ = !inSingleQ;
                    else if (!inSingleQ && ch == '"' && !inFormat)
                        inDoubleQ = !inDoubleQ;
                    bool insideStr = inSingleQ || inDoubleQ;

                    if (!insideStr)
                    {
                        if (ch == '(')
                            parenDepth++;
                        else if (ch == ')')
                        {
                            if (parenDepth > 0)
                                parenDepth--;
                        }
                        else if (ch == '{')
                            depth++;
                        else if (ch == '}')
                        {
                            depth--;
                            if (depth == 0)
                            {
                                advance();
                                break;
                            }
                        }
                        // Only treat ':' as format separator at top-level (no nested parens/braces)
                        if (depth == 1 && parenDepth == 0 && ch == ':' && !inFormat)
                        {
                            inFormat = true;
                            advance();
                            continue;
                        }
                    }

                    if (inFormat)
                        fmtPart += ch;
                    else
                        exprPart += ch;
                    advance();
                }

                if (inFormat)
                {
                    raw += "${__format__(" + exprPart + ", \"" + fmtPart + "\")}";
                }
                else
                {
                    raw += "${" + exprPart + "}";
                }
            }
            else if (current() == '\\')
            {
                raw += current();
                advance();
                if (pos < src.size())
                {
                    raw += current();
                    advance();
                }
            }
            else
            {
                raw += current();
                advance();
            }
        }
        if (pos < src.size())
            advance(); // skip closing quote
        // Re-lex wrapped in backticks using the existing template literal engine
        std::string backtickSrc = "`" + raw + "`";
        Lexer subLex(backtickSrc);
        // We can't return multiple tokens from here — caller handles this
        // Store pending tokens and return a sentinel; instead, use a different approach:
        // Return a special STRING token with the expanded value by evaluating immediately
        // Actually the cleanest: tokenize the backtick source and push into the caller's stream
        // Since we can't do that from here, return a placeholder and let tokenize() handle it
        // INSTEAD: we set a member flag with the pending tokens
        pendingTokens_ = subLex.tokenize();
        // Remove EOF from pending
        if (!pendingTokens_.empty() && pendingTokens_.back().type == TokenType::EOF_TOKEN)
            pendingTokens_.pop_back();
        // Return a dummy that tokenize() will replace with pendingTokens_
        return Token(TokenType::UNKNOWN, "__fstring__", startLine, startCol);
    }

    auto it = keywords.find(id);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;

    // C preprocessor macro expansion: if this identifier was #defined, substitute it
    auto dit = defines_.find(id);
    if (dit != defines_.end() && !dit->second.empty())
    {
        // Single-token macro (most common: #define ROWS 18, #define RIGHT 3)
        if (dit->second.size() == 1)
            return dit->second[0]; // return the replacement token directly
        // Multi-token macro: push all but first into pendingTokens_
        pendingTokens_.insert(pendingTokens_.begin(),
                              dit->second.begin() + 1,
                              dit->second.end());
        return dit->second[0];
    }

    return Token(type, id, startLine, startCol);
}
```

## Line-by-Line Analysis

### **Function Signature and Initialization (Lines 1-5)**
```cpp
Token Lexer::readIdentifierOrKeyword()
{
    int startLine = line, startCol = col;
    std::string id;
    while (pos < src.size() && (std::isalnum(current()) || current() == '_'))
        id += advance();
```

**Initialization Analysis:**
- **`Token` Return Type**: Returns IDENTIFIER or keyword token
- **Position Tracking**: Records starting position for error reporting
- **Identifier Building**: `id` string accumulates the identifier text
- **Character Collection**: Continues while characters are alphanumeric or underscore

### **Raw String Processing (Lines 7-19)**
```cpp
// Raw string prefix: r"..." or r'...' — literal string with no escape sequences
if ((id == "r" || id == "R") && pos < src.size() && (current() == '"' || current() == '\''))
{
    char quote = current();
    int strStartLine = line, strStartCol = col;
    advance(); // skip opening quote
    std::string raw;
    while (pos < src.size() && current() != quote)
    {
        raw += advance();
    }
    if (pos < src.size())
        advance(); // skip closing quote
    return Token(TokenType::STRING, raw, strStartLine, strStartCol);
}
```

**Raw String Features:**
- **Prefix Detection**: Checks for `r` or `R` followed by quote
- **No Escape Processing**: Characters taken literally
- **Quote Support**: Works with both single and double quotes
- **Direct Return**: Returns STRING token immediately

### **F-String Processing (Lines 21-89)**
```cpp
// f-string prefix: f"..." or f'...'  — treat like a backtick template literal
if ((id == "f" || id == "F") && pos < src.size() && (current() == '"' || current() == '\''))
{
    // Complex f-string processing with format specifiers
    // Converts {expr:format} → ${__format__(expr, "format")}
}
```

**F-String Features:**
- **Prefix Detection**: Checks for `f` or `F` followed by quote
- **Expression Parsing**: Extracts expressions from `{}` blocks
- **Format Specifiers**: Handles `{expr:format}` syntax
- **Template Conversion**: Converts to backtick template format
- **Pending Tokens**: Stores processed tokens for later emission

### **Keyword Lookup (Lines 91-94)**
```cpp
auto it = keywords.find(id);
TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
```

**Keyword Detection:**
- **Hash Table Lookup**: Uses unordered_map for O(1) keyword detection
- **Fallback to Identifier**: Non-keywords become IDENTIFIER tokens
- **Case Sensitive**: Keyword matching is case-sensitive
- **Efficient**: Constant-time lookup for all keywords

### **Macro Expansion (Lines 96-112)**
```cpp
// C preprocessor macro expansion: if this identifier was #defined, substitute it
auto dit = defines_.find(id);
if (dit != defines_.end() && !dit->second.empty())
{
    // Single-token macro (most common: #define ROWS 18, #define RIGHT 3)
    if (dit->second.size() == 1)
        return dit->second[0]; // return the replacement token directly
    // Multi-token macro: push all but first into pendingTokens_
    pendingTokens_.insert(pendingTokens_.begin(),
                          dit->second.begin() + 1,
                          dit->second.end());
    return dit->second[0];
}
```

**Macro Features:**
- **Macro Lookup**: Checks if identifier is a defined macro
- **Single Token Macros**: Direct replacement for simple macros
- **Multi-Token Macros**: Pending token mechanism for complex macros
- **C Compatibility**: Supports C-style #define macros

### **Token Creation (Line 114)**
```cpp
return Token(type, id, startLine, startCol);
```

**Token Construction:**
- **Dynamic Type**: Either keyword token type or IDENTIFIER
- **Original Text**: Preserves original identifier text
- **Position Information**: Starting position for error reporting
- **Clean Return**: Properly formed token for parser

## Design Patterns and Architecture

### **Multi-Stage Processing Pattern**
```cpp
Token readIdentifierOrKeyword() {
    // Stage 1: Read identifier text
    std::string id = readIdentifierText();
    
    // Stage 2: Special cases (raw strings, f-strings)
    if (isRawString(id)) return processRawString();
    if (isFString(id)) return processFString();
    
    // Stage 3: Keyword lookup
    TokenType type = lookupKeyword(id);
    
    // Stage 4: Macro expansion
    if (isMacro(id)) return expandMacro(id);
    
    return createToken(type, id);
}
```
- **Sequential Processing**: Each stage processes specific cases
- **Early Returns**: Special cases return immediately
- **Fallback Logic**: General case handled at end
- **Extensible**: Easy to add new processing stages

### **Strategy Pattern**
```cpp
// Different processing strategies based on identifier
if (id == "r" || id == "R") {
    return processRawStringStrategy();
} else if (id == "f" || id == "F") {
    return processFStringStrategy();
} else {
    return processIdentifierStrategy();
}
```
- **Context-Sensitive Processing**: Different behavior for different identifiers
- **Strategy Selection**: Clear selection logic
- **Isolated Logic**: Each strategy handled separately
- **Maintainable**: Easy to modify individual strategies

### **Lookup Pattern**
```cpp
// Keyword lookup
auto it = keywords.find(id);
TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;

// Macro lookup
auto dit = defines_.find(id);
if (dit != defines_.end()) {
    // Handle macro expansion
}
```
- **Hash Table Lookup**: O(1) lookup using unordered_map
- **Fallback Handling**: Default behavior when not found
- **Multiple Lookups**: Separate lookups for different purposes
- **Efficient**: Constant-time performance

## Technical Considerations

### **Character Classification**
```cpp
std::isalnum(current()) || current() == '_'
```

**Identifier Rules:**
- **Alphanumeric**: Letters and digits (after first character)
- **Underscore**: Underscore allowed anywhere
- **First Character**: Must be letter or underscore (implicit)
- **Standard Rules**: Matches common programming language conventions

### **Raw String Processing**
```cpp
r"Hello, World!"  // Raw string with no escape sequences
r'Hello, World!'  // Raw string with single quotes
```

**Raw String Features:**
- **No Escapes**: Backslashes treated literally
- **Cross-Platform**: Works on all platforms
- **Regex Friendly**: Ideal for regular expressions
- **Path Handling**: Useful for file paths

### **F-String Processing**
```cpp
f"Hello {name}, age {age}"           // Simple f-string
f"Value: {value:.2f}"               // With format specifier
f"{name} is {age} years old"        // Multiple expressions
```

**F-String Features:**
- **Expression Interpolation**: `{expression}` syntax
- **Format Specifiers**: `{expr:format}` syntax
- **Template Conversion**: Converts to backtick templates
- **Complex Parsing**: Handles nested braces and strings

### **Macro Expansion**
```cpp
#define PI 3.14159           // Single-token macro
#define MAX(a,b) ((a) > (b) ? (a) : (b))  // Multi-token macro
```

**Macro Features:**
- **C Compatibility**: Standard C preprocessor syntax
- **Token Replacement**: Substitutes macros with replacement tokens
- **Multi-Token Support**: Handles complex macro expansions
- **Pending Tokens**: Mechanism for multi-token macros

## Usage Patterns

### **Basic Identifier Recognition**
```cpp
Token readToken() {
    if (std::isalpha(current()) || current() == '_') {
        return readIdentifierOrKeyword();
    }
    // ... other token types
}
```

### **Keyword Detection**
```cpp
// Input: "if", "while", "function"
// Output: IF, WHILE, FUNCTION tokens
```

### **Raw String Usage**
```cpp
// In source code:
path = r"C:\Users\name\file.txt"  // Raw string
regex = r'\d+\.\d+'                // Raw regex pattern
```

### **F-String Usage**
```cpp
// In source code:
name = f"User {user_id}"           // Simple interpolation
value = f"Result: {result:.2f}"     // With formatting
message = f"{name} is {age}"        // Multiple expressions
```

## Error Handling and Edge Cases

### **Empty Identifier**
```cpp
// Input: "_" (just underscore)
// Result: IDENTIFIER token with value "_"
```

### **Invalid Raw String**
```cpp
// Input: "r" followed by non-quote character
// Result: IDENTIFIER token with value "r"
```

### **Invalid F-String**
```cpp
// Input: "f" followed by non-quote character
// Result: IDENTIFIER token with value "f"
```

### **Unterminated Strings**
```cpp
// Input: r"unclosed string
// Error: Detected in string processing logic
```

### **Malformed F-Strings**
```cpp
// Input: f"unclosed {expression
// Error: Detected in f-string processing logic
```

## Performance Optimization

### **Character Classification Optimization**
```cpp
// Fast identifier character checking
bool isIdentifierChar(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}
```

### **String Building Optimization**
```cpp
// Reserve space for common identifier lengths
id.reserve(32);  // Most identifiers fit in 32 characters
while (isIdentifierChar(current())) {
    id += advance();
}
```

### **Hash Table Optimization**
```cpp
// Pre-reserve space for keyword table
keywords.reserve(100);  // Estimated keyword count
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Position Tracking**
```cpp
// Correct: Record start before reading identifier
int startLine = line, startCol = col;
while (condition) {
    id += advance();
}

// Wrong: Recording after reading identifier
while (condition) {
    id += advance();
}
int startLine = line, startCol = col;  // Points to end, not start
```

#### **F-String Complexity**
```cpp
// Complex f-string parsing requires careful state tracking
bool inFormat = false;
int depth = 1;
int parenDepth = 0;
// Must track multiple nested structures
```

### **Debugging Support**
```cpp
Token Lexer::readIdentifierOrKeyword() {
    int startLine = line, startCol = col;
    std::string id;
    
    while (pos < src.size() && (std::isalnum(current()) || current() == '_')) {
        id += advance();
    }
    
    if (getenv("DEBUG_IDENTIFIERS")) {
        std::cout << "Read identifier: '" << id << "' at " 
                  << startLine << ":" << startCol << "\n";
        
        if (isKeyword(id)) {
            std::cout << "  -> Keyword: " << tokenTypeToString(keywords.at(id)) << "\n";
        } else if (isMacro(id)) {
            std::cout << "  -> Macro with " << defines_[id].size() << " tokens\n";
        } else {
            std::cout << "  -> Identifier\n";
        }
    }
    
    // ... rest of function
}
```

### **Validation Testing**
```cpp
void testIdentifierParsing() {
    struct TestCase {
        std::string input;
        TokenType expectedType;
        std::string expectedValue;
    };
    
    std::vector<TestCase> tests = {
        {"variable", TokenType::IDENTIFIER, "variable"},
        {"if", TokenType::IF, "if"},
        {"function", TokenType::FUNCTION, "function"},
        {"r\"raw\"", TokenType::STRING, "raw"},
        {"f\"{expr}\"", TokenType::UNKNOWN, "__fstring__"}
    };
    
    for (const auto &test : tests) {
        Lexer lexer(test.input);
        Token token = lexer.readIdentifierOrKeyword();
        assert(token.type == test.expectedType);
        assert(token.value == test.expectedValue);
    }
}
```

## Extension Points

### **Unicode Identifiers**
```cpp
bool isUnicodeIdentifierStart(char32_t c) {
    // Support Unicode identifier start characters
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c == '_') ||
           (c >= 0x00C0 && c <= 0x00D6) ||  // Latin-1 supplements
           (c >= 0x00D8 && c <= 0x00F6);   // More Unicode ranges
}
```

### **Custom String Prefixes**
```cpp
// Add new string types
if (id == "b" && isQuote(current())) {
    return readBinaryString(current());
} else if (id == "u8" && isQuote(current())) {
    return readUtf8String(current());
}
```

### **Enhanced Macro System**
```cpp
// Function-like macros with parameters
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// Would need parameter parsing and substitution
Token expandFunctionMacro(const std::string &name, 
                          const std::vector<std::string> &args);
```

### **Identifier Validation**
```cpp
class IdentifierValidator {
public:
    bool isValid(const std::string &id) {
        if (id.empty()) return false;
        if (!isIdentifierStart(id[0])) return false;
        for (char c : id) {
            if (!isIdentifierChar(c)) return false;
        }
        return !isReservedWord(id);
    }
    
    std::string normalize(const std::string &id) {
        // Apply naming conventions
        return toCamelCase(id);
    }
};
```

## Advanced Usage Examples

### **Complex F-String Processing**
```cpp
// Input: f"Value: {value:.2f} (precision: 2)"
// Output: Template: "Value: " + str(value, "%.2f") + " (precision: 2)"
```

### **Nested Macro Expansion**
```cpp
#define PI 3.14159
#define TWO_PI (2 * PI)

// When TWO_PI is encountered:
// 1. Lookup TWO_PI macro
// 2. Find PI macro in replacement
// 3. Expand PI first, then TWO_PI
```

### **String Type Detection**
```cpp
enum class StringType {
    REGULAR,
    RAW,
    F_STRING,
    UTF8,
    BINARY
};

StringType detectStringType(const std::string &prefix) {
    if (prefix == "r" || prefix == "R") return StringType::RAW;
    if (prefix == "f" || prefix == "F") return StringType::F_STRING;
    if (prefix == "u8") return StringType::UTF8;
    if (prefix == "b") return StringType::BINARY;
    return StringType::REGULAR;
}
```

## Why This Design Works

### **Flexibility**
- **Multiple String Types**: Raw strings, f-strings, regular strings
- **Macro Support**: C-style preprocessor compatibility
- **Keyword Detection**: Efficient hash table lookup
- **Extensible**: Easy to add new string prefixes or features

### **Performance**
- **Efficient Lookups**: O(1) keyword and macro detection
- **Early Returns**: Special cases handled immediately
- **Minimal Overhead**: Simple character classification
- **Optimized Building**: String accumulation with minimal allocation

### **Robustness**
- **Comprehensive Coverage**: Handles all identifier cases
- **Error Prevention**: Proper bounds checking and validation
- **Graceful Fallbacks**: Default behavior for unknown cases
- **State Management**: Proper handling of complex parsing states

### **Maintainability**
- **Clear Structure**: Logical separation of different processing cases
- **Self-Documenting**: Function and variable names are clear
- **Modular Design**: Each feature handled in separate section
- **Testable**: Each component can be tested independently

The `readIdentifierOrKeyword()` function demonstrates sophisticated lexical analyzer design, providing comprehensive identifier and keyword processing with advanced features like raw strings, f-strings, and macro expansion while maintaining excellent performance and extensibility.
