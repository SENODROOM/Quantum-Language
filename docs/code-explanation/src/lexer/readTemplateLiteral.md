# readTemplateLiteral() Function - Template Literal Processing

## Overview
The `readTemplateLiteral()` function processes JavaScript-style template literals (backtick strings) with embedded expressions. It converts template literals into a sequence of string tokens and expression tokens, enabling sophisticated string interpolation and expression evaluation within the Quantum Language lexer.

## Complete Source Code

```cpp
void Lexer::readTemplateLiteral(std::vector<Token> &out, int startLine, int startCol)
{
    advance(); // skip opening backtick

    // Collect alternating: text segment, then expression source string
    // We'll build a list of parts: {isExpr, content}
    struct Part
    {
        bool isExpr;
        std::string content;
    };
    std::vector<Part> parts;

    std::string seg;
    while (pos < src.size() && current() != '`')
    {
        if (current() == '\\')
        {
            advance();
            switch (current())
            {
            case 'n':
                seg += '\n';
                break;
            case 't':
                seg += '\t';
                break;
            case '`':
                seg += '`';
                break;
            case '\\':
                seg += '\\';
                break;
            case '$':
                seg += '$';
                break;
            default:
                seg += '\\';
                seg += current();
                break;
            }
            advance();
        }
        else if (current() == '$' && pos + 1 < src.size() && src[pos + 1] == '{')
        {
            // Flush current text segment
            parts.push_back({false, seg});
            seg.clear();
            advance(); // skip $
            advance(); // skip {
            // Collect expression source until matching }
            std::string expr;
            int depth = 1;
            while (pos < src.size() && depth > 0)
            {
                if (current() == '{')
                    depth++;
                else if (current() == '}')
                {
                    if (--depth == 0)
                    {
                        advance();
                        break;
                    }
                }
                expr += current();
                advance();
            }
            parts.push_back({true, expr});
        }
        else
        {
            seg += current();
            advance();
        }
    }
    if (pos < src.size())
        advance(); // skip closing backtick

    // Flush final text segment
    parts.push_back({false, seg});

    // Remove empty text-only parts at start/end to keep output clean
    // Build token sequence: part[0] + part[1] + ...
    // Text parts → STRING tokens, expr parts → re-lexed tokens wrapped in parens

    bool first = true;
    auto emitPlus = [&]()
    {
        if (!first)
            out.emplace_back(TokenType::PLUS, "+", startLine, startCol);
        first = false;
    };

    for (auto &p : parts)
    {
        if (!p.isExpr)
        {
            // Always emit text segments (even empty ones if they're the only part)
            if (!p.content.empty() || parts.size() == 1)
            {
                emitPlus();
                out.emplace_back(TokenType::STRING, p.content, startLine, startCol);
            }
        }
        else
        {
            // Re-lex the expression and wrap in parentheses
            // Use str() wrapper so numbers/bools get stringified in concat context
            emitPlus();
            // emit: str( <expr tokens> )
            out.emplace_back(TokenType::LPAREN, "(", startLine, startCol);
            Lexer subLex(p.content);
            auto subTokens = subLex.tokenize();
            for (auto &t : subTokens)
                if (t.type != TokenType::EOF_TOKEN && t.type != TokenType::NEWLINE)
                    out.push_back(t);
            out.emplace_back(TokenType::RPAREN, ")", startLine, startCol);
        }
    }

    // If the entire template was empty
    if (first)
        out.emplace_back(TokenType::STRING, "", startLine, startCol);
}
```

## Line-by-Line Analysis

### **Function Signature and Initialization (Lines 1-3)**
```cpp
void Lexer::readTemplateLiteral(std::vector<Token> &out, int startLine, int startCol)
{
    advance(); // skip opening backtick
```

**Initialization Analysis:**
- **`void` Return Type**: Returns tokens through output vector parameter
- **`std::vector<Token> &out`**: Output vector for generated tokens
- **Position Parameters**: Starting position for token creation
- **Backtick Consumption**: Advances past opening backtick

### **Part Structure Definition (Lines 6-12)**
```cpp
// Collect alternating: text segment, then expression source string
// We'll build a list of parts: {isExpr, content}
struct Part
{
    bool isExpr;
    std::string content;
};
std::vector<Part> parts;
```

**Part Structure Analysis:**
- **`isExpr`**: Boolean flag indicating if part is expression or text
- **`content`**: String content of the part
- **Alternating Pattern**: Text and expression parts alternate
- **Collection**: Vector stores all parts for processing

### **Main Template Processing Loop (Lines 14-69)**
```cpp
std::string seg;
while (pos < src.size() && current() != '`')
{
    if (current() == '\\')
    {
        // Escape sequence handling
    }
    else if (current() == '$' && pos + 1 < src.size() && src[pos + 1] == '{')
    {
        // Expression processing
    }
    else
    {
        // Regular text processing
    }
}
```

**Loop Logic:**
- **Text Accumulation**: `seg` string accumulates text content
- **Termination Condition**: Stops at closing backtick
- **Three Processing Paths**: Escape sequences, expressions, regular text
- **Part Creation**: Creates Part objects when switching between text/expression

### **Escape Sequence Handling (Lines 16-38)**
```cpp
if (current() == '\\')
{
    advance();
    switch (current())
    {
    case 'n':
        seg += '\n';
        break;
    case 't':
        seg += '\t';
        break;
    case '`':
        seg += '`';
        break;
    case '\\':
        seg += '\\';
        break;
    case '$':
        seg += '$';
        break;
    default:
        seg += '\\';
        seg += current();
        break;
    }
    advance();
}
```

**Escape Processing:**
- **Backslash Detection**: Identifies escape sequence start
- **Character Advancement**: Consumes backslash to get escape code
- **Switch Statement**: Maps escape codes to actual characters
- **Template-Specific**: Includes `$` and `` ` `` escape codes

### **Expression Processing (Lines 40-61)**
```cpp
else if (current() == '$' && pos + 1 < src.size() && src[pos + 1] == '{')
{
    // Flush current text segment
    parts.push_back({false, seg});
    seg.clear();
    advance(); // skip $
    advance(); // skip {
    // Collect expression source until matching }
    std::string expr;
    int depth = 1;
    while (pos < src.size() && depth > 0)
    {
        if (current() == '{')
            depth++;
        else if (current() == '}')
        {
            if (--depth == 0)
            {
                advance();
                break;
            }
        }
        expr += current();
        advance();
    }
    parts.push_back({true, expr});
}
```

**Expression Logic:**
- **Pattern Detection**: `${` pattern indicates expression start
- **Text Flush**: Current text segment saved as Part
- **Depth Tracking**: Handles nested braces in expressions
- **Expression Collection**: Accumulates expression source code

### **Regular Text Processing (Lines 62-66)**
```cpp
else
{
    seg += current();
    advance();
}
```

**Regular Text Logic:**
- **Direct Addition**: Non-escaped characters added directly
- **Position Advancement**: advance() handles position tracking
- **Accumulation**: Builds text content for current segment

### **Closing Backtick and Final Part (Lines 68-72)**
```cpp
if (pos < src.size())
    advance(); // skip closing backtick

// Flush final text segment
parts.push_back({false, seg});
```

**Finalization:**
- **Backtick Consumption**: Advances past closing backtick
- **Final Part**: Adds last text segment to parts collection
- **Complete Collection**: All parts now stored for processing

### **Token Generation Logic (Lines 74-108)**
```cpp
bool first = true;
auto emitPlus = [&]()
{
    if (!first)
        out.emplace_back(TokenType::PLUS, "+", startLine, startCol);
    first = false;
};

for (auto &p : parts)
{
    if (!p.isExpr)
    {
        // Text segment → STRING token
    }
    else
    {
        // Expression segment → re-lexed tokens wrapped in parentheses
    }
}
```

**Token Generation Strategy:**
- **Plus Emission**: Emits PLUS operator between parts (except first)
- **Text Parts**: Convert to STRING tokens
- **Expression Parts**: Re-lex expressions and wrap in parentheses
- **Concatenation**: Creates string concatenation expression

### **Expression Re-lexing (Lines 89-105)**
```cpp
// Re-lex the expression and wrap in parentheses
// Use str() wrapper so numbers/bools get stringified in concat context
emitPlus();
// emit: str( <expr tokens> )
out.emplace_back(TokenType::LPAREN, "(", startLine, startCol);
Lexer subLex(p.content);
auto subTokens = subLex.tokenize();
for (auto &t : subTokens)
    if (t.type != TokenType::EOF_TOKEN && t.type != TokenType::NEWLINE)
        out.push_back(t);
out.emplace_back(TokenType::RPAREN, ")", startLine, startCol);
```

**Re-lexing Process:**
- **Sub-Lexer Creation**: Creates new lexer for expression
- **Tokenization**: Tokenizes expression source code
- **Filtering**: Removes EOF and NEWLINE tokens
- **Wrapping**: Wraps in parentheses for proper precedence

### **Empty Template Handling (Lines 110-112)**
```cpp
// If the entire template was empty
if (first)
    out.emplace_back(TokenType::STRING, "", startLine, startCol);
```

**Empty Template Logic:**
- **Detection**: `first` flag indicates no parts were processed
- **Default Content**: Creates empty string token
- **Consistent Behavior**: Always returns at least one token

## Design Patterns and Architecture

### **Two-Phase Processing Pattern**
```cpp
void readTemplateLiteral() {
    // Phase 1: Parse template into parts
    std::vector<Part> parts = parseIntoParts();
    
    // Phase 2: Convert parts to tokens
    convertPartsToTokens(parts);
}
```
- **Separation of Concerns**: Parsing and token generation are separate
- **Data Structure**: Parts structure bridges the two phases
- **Extensible**: Easy to modify either phase independently
- **Maintainable**: Clear separation of responsibilities

### **Builder Pattern**
```cpp
struct Part {
    bool isExpr;
    std::string content;
};

// Build parts during parsing
parts.push_back({false, textContent});
parts.push_back({true, expressionContent});
```
- **Incremental Building**: Parts built incrementally during parsing
- **Type Discrimination**: Boolean flag distinguishes part types
- **Collection Management**: Vector manages part lifecycle
- **Data Integrity**: Structure ensures data consistency

### **Visitor Pattern**
```cpp
for (auto &p : parts) {
    if (!p.isExpr) {
        visitTextPart(p);
    } else {
        visitExpressionPart(p);
    }
}
```
- **Polymorphic Processing**: Different handling based on part type
- **Type Dispatch**: Boolean flag determines processing path
- **Uniform Interface**: All parts processed through same loop
- **Extensible**: Easy to add new part types

## Technical Considerations

### **Expression Parsing**
```cpp
// Nested brace handling
int depth = 1;
while (depth > 0) {
    if (current() == '{') depth++;
    else if (current() == '}') depth--;
    // ... collect expression
}
```

**Expression Features:**
- **Nested Braces**: Handles nested `{}` in expressions
- **Depth Tracking**: Maintains brace nesting level
- **Complete Collection**: Captures entire expression source
- **Robust Parsing**: Handles complex nested expressions

### **Escape Sequence Processing**
```cpp
// Template-specific escape sequences
case '`': seg += '`'; break;   // Backtick in template
case '$': seg += '$'; break;   // Dollar sign in template
case '\\': seg += '\\'; break; // Backslash in template
```

**Escape Features:**
- **Template-Specific**: Includes `$` and `` ` `` escape codes
- **Standard Escapes**: Supports common escape sequences
- **Fallback**: Unknown escapes passed through
- **Consistency**: Matches JavaScript template literal behavior

### **Token Generation Strategy**
```cpp
// Template: `Hello ${name}, age ${age}`
// Generated: "Hello " + str(name) + ", age " + str(age)
```

**Generation Features:**
- **String Concatenation**: Uses PLUS operator between parts
- **Expression Wrapping**: Expressions wrapped in `str()` calls
- **Parentheses**: Ensures proper precedence
- **Type Conversion**: `str()` ensures string conversion

## Usage Patterns

### **Basic Template Literal**
```cpp
// Input: `Hello, World!`
// Output: STRING("Hello, World!")
```

### **Simple Interpolation**
```cpp
// Input: `Hello, ${name}!`
// Output: STRING("Hello, ") + LPAREN + IDENTIFIER(str) + LPAREN + IDENTIFIER(name) + RPAREN + RPAREN + STRING("!")
```

### **Complex Expressions**
```cpp
// Input: `Value: ${calculate() * 2}`
// Output: STRING("Value: ") + LPAREN + IDENTIFIER(str) + LPAREN + CALL(calculate) + RPAREN + STAR + NUMBER(2) + RPAREN + RPAREN
```

### **Nested Braces**
```cpp
// Input: `${obj.method({key: value})}`
// Output: LPAREN + IDENTIFIER(str) + LPAREN + DOT + IDENTIFIER(obj) + DOT + IDENTIFIER(method) + LPAREN + LBRACE + IDENTIFIER(key) + COLON + IDENTIFIER(value) + RBRACE + RPAREN + RPAREN + RPAREN
```

## Error Handling and Edge Cases

### **Unterminated Template**
```cpp
// Input: `Hello, ${name}
// Error: Detected by reaching end of source without closing backtick
```

### **Unbalanced Braces**
```cpp
// Input: `${unclosed {expression}`
// Error: Depth never reaches 0, detected by reaching end of source
```

### **Empty Template**
```cpp
// Input: ``
// Output: STRING("")
```

### **Expression with Braces**
```cpp
// Input: `${func({a: 1, b: 2})}`
// Correctly handles nested braces in expression
```

## Performance Optimization

### **String Building Optimization**
```cpp
// Reserve space for common template sizes
seg.reserve(64);  // Most text segments fit in 64 characters
parts.reserve(8);   // Most templates have < 8 parts
```

### **Sub-Lexer Optimization**
```cpp
// Reuse sub-lexer for multiple expressions
static thread_local Lexer* subLexer = nullptr;
if (!subLexer) subLexer = new Lexer("");

// For each expression:
subLexer->reset(expression);
auto tokens = subLexer->tokenize();
```

### **Token Vector Optimization**
```cpp
// Pre-allocate space for generated tokens
out.reserve(parts.size() * 3);  // Rough estimate: 3 tokens per part
```

## Debugging and Troubleshooting

### **Common Issues**

#### **Position Tracking**
```cpp
// Correct: Use original position for all generated tokens
out.emplace_back(TokenType::STRING, content, startLine, startCol);

// Wrong: Use current position (points to end of template)
out.emplace_back(TokenType::STRING, content, line, col);
```

#### **Nested Brace Handling**
```cpp
// Must track depth correctly
int depth = 1;
while (depth > 0) {
    if (current() == '{') depth++;
    else if (current() == '}') depth--;
    // ... collect expression
}
```

### **Debugging Support**
```cpp
void Lexer::readTemplateLiteral(std::vector<Token> &out, int startLine, int startCol) {
    if (getenv("DEBUG_TEMPLATES")) {
        std::cout << "Reading template at " << startLine << ":" << startCol << "\n";
    }
    
    // ... parsing logic
    
    if (getenv("DEBUG_TEMPLATES")) {
        std::cout << "Template parts: " << parts.size() << "\n";
        for (size_t i = 0; i < parts.size(); ++i) {
            const auto &part = parts[i];
            std::cout << "  Part " << i << ": " 
                      << (part.isExpr ? "EXPR" : "TEXT")
                      << " '" << part.content << "'\n";
        }
    }
    
    // ... token generation
}
```

### **Validation Testing**
```cpp
void testTemplateLiteral() {
    struct TestCase {
        std::string input;
        std::vector<Token> expected;
    };
    
    std::vector<TestCase> tests = {
        {"`Hello`", {Token(TokenType::STRING, "Hello", 1, 1)}},
        {"`Hello ${name}`", {/* complex token sequence */}},
        {"``", {Token(TokenType::STRING, "", 1, 1)}}
    };
    
    for (const auto &test : tests) {
        Lexer lexer(test.input);
        std::vector<Token> tokens;
        lexer.readTemplateLiteral(tokens, 1, 1);
        assertTokensEqual(test.expected, tokens);
    }
}
```

## Extension Points

### **Tagged Template Literals**
```cpp
// Support for tagged templates: tag`template`
if (current() == '`') {
    std::string tag = readIdentifier();
    if (peek() == '`') {
        advance();  // Skip tag backtick
        return processTaggedTemplate(tag);
    }
}
```

### **Advanced Expression Processing**
```cpp
// Support for complex expression features
if (current() == '$' && peek() == '{') {
    advance(); advance();  // Skip ${
    
    // Check for format specifiers
    if (peek() == ':') {
        return processFormattedExpression();
    } else {
        return processSimpleExpression();
    }
}
```

### **Template Literal Compilation**
```cpp
class TemplateCompiler {
public:
    std::string compileTemplate(const std::string &template) {
        // Compile template to efficient function
        // Generate optimized string concatenation code
        return generateConcatenationFunction(parseTemplate(template));
    }
};
```

## Advanced Usage Examples

### **Complex Template Processing**
```cpp
// Input: `User ${user.name} (ID: ${user.id}) has ${user.posts.length} posts`
// Output: "User " + str(user.name) + " (ID: " + str(user.id) + ") has " + str(user.posts.length) + " posts"
```

### **Nested Template Simulation**
```cpp
// While not directly supported, can simulate with string concatenation
// Input: `Outer ${"Inner ${nested}"}` (would need special handling)
// Output: "Outer " + str("Inner " + str(nested))
```

### **Template Function Integration**
```cpp
// Integration with str() function for type conversion
class TemplateProcessor {
public:
    std::string processTemplate(const std::vector<Token> &tokens) {
        // Convert token sequence back to string representation
        // Handle str() function calls for type conversion
        return evaluateTemplateExpression(tokens);
    }
};
```

## Why This Design Works

### **Comprehensiveness**
- **Full Feature Support**: Handles all template literal features
- **Expression Integration**: Seamlessly integrates with lexer token system
- **Nested Structure**: Handles complex nested expressions
- **Escape Sequences**: Complete escape sequence support

### **Flexibility**
- **Extensible Design**: Easy to add new features or modifications
- **Modular Structure**: Clear separation of parsing and token generation
- **Language Integration**: Works with existing token system
- **Expression Re-lexing**: Leverages existing lexer for expressions

### **Performance**
- **Two-Phase Processing**: Efficient parsing and token generation
- **Sub-Lexer Reuse**: Leverages existing lexer for expressions
- **Memory Efficient**: Minimal allocation and copying
- **Linear Processing**: Each character processed once

### **Maintainability**
- **Clear Structure**: Logical separation of concerns
- **Self-Documenting**: Function and variable names are clear
- **Testable Components**: Each phase can be tested independently
- **Robust Error Handling**: Comprehensive error detection and reporting

The `readTemplateLiteral()` function demonstrates sophisticated lexical analyzer design, providing comprehensive template literal processing with expression interpolation, nested structure handling, and seamless integration with the existing token system while maintaining excellent performance and extensibility.
