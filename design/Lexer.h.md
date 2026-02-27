# `Lexer.h` Line-by-Line Explanation

Source file: `include/Lexer.h`

## Whole file explanation

`Lexer.h` is the interface (blueprint) for the lexer component of your language.

At a high level, this header defines:
- What the lexer does: convert raw source code text into a list of tokens.
- What outside code can call: construct a `Lexer` with source text, then call `tokenize()`.
- What internal state it keeps while scanning: current position, line, and column.
- What helper operations it uses internally: character movement, whitespace/comment skipping, and token readers.

Conceptually, the flow is:
1. Store the input source in `src`.
2. Start scanning from `pos = 0`.
3. Repeatedly inspect current characters with `current()` and `peek()`.
4. Move through text with `advance()`, updating line/column counters.
5. Ignore irrelevant text (`skipWhitespace()`, `skipComment()`).
6. Build concrete tokens using specialized readers (`readNumber()`, `readString()`, etc.).
7. Return all produced tokens from `tokenize()`.

The `keywords` map is important because identifiers and keywords look similar at first (`if`, `while`, `name`, `count` are all text). The lexer first reads identifier-like text, then checks if that text is a reserved keyword.

This file is intentionally only a header, so it declares behavior but does not implement it. Implementations live in the corresponding `.cpp` file.

## Line-by-line

### Line 1
```cpp
#pragma once
```
Preprocessor guard that prevents this header from being included multiple times in one translation unit.

### Line 2
```cpp
#include "Token.h"
```
Includes token definitions (`Token`, `TokenType`) used by the lexer interface.

### Line 3
```cpp
#include <string>
```
Needed for `std::string` (source code storage and text processing).

### Line 4
```cpp
#include <vector>
```
Needed for `std::vector<Token>` returned by `tokenize()`.

### Line 5
```cpp
#include <unordered_map>
```
Needed for the keyword lookup table (`keywords`).

### Line 6
```cpp

```
Blank line for readability; separates includes from class declaration.

### Line 7
```cpp
class Lexer {
```
Declares the `Lexer` class, responsible for converting source text into tokens.

### Line 8
```cpp
public:
```
Starts the public API section (what outside code can use).

### Line 9
```cpp
explicit Lexer(const std::string& source);
```
Constructor that receives source code text.
- `explicit` prevents accidental implicit conversion from `std::string` to `Lexer`.
- `const std::string&` avoids copying on input.

### Line 10
```cpp
std::vector<Token> tokenize();
```
Main lexer entry point. Scans the full source and returns a list of tokens.

### Line 11
```cpp

```
Blank line separating public API from private internals.

### Line 12
```cpp
private:
```
Starts private section (internal state and helper functions).

### Line 13
```cpp
std::string src;
```
Stores the full input source code being lexed.

### Line 14
```cpp
size_t pos;
```
Current character index in `src`.

### Line 15
```cpp
int line, col;
```
Tracks current line and column for diagnostics and token positions.

### Line 16
```cpp

```
Blank line separating state fields from static lookup data.

### Line 17
```cpp
static const std::unordered_map<std::string, TokenType> keywords;
```
Static keyword table mapping text (like `"if"`, `"while"`) to `TokenType`.
- `static`: shared by all `Lexer` instances.
- `const`: read-only after definition.

### Line 18
```cpp

```
Blank line separating data members from helper methods.

### Line 19
```cpp
char current() const;
```
Returns character at current position without moving forward.

### Line 20
```cpp
char peek(int offset = 1) const;
```
Looks ahead by `offset` characters (default one char ahead), without consuming input.

### Line 21
```cpp
char advance();
```
Consumes current character and moves cursor forward (usually updates `pos`, `line`, `col`).

### Line 22
```cpp
void skipWhitespace();
```
Skips spaces/tabs/newlines that are not semantically meaningful tokens.

### Line 23
```cpp
void skipComment();
```
Skips comment text so comments do not become tokens.

### Line 24
```cpp

```
Blank line separating low-level movement helpers from token readers.

### Line 25
```cpp
Token readNumber();
```
Reads a numeric literal from current position and returns a number token.

### Line 26
```cpp
Token readString(char quote);
```
Reads a string literal using the opening quote character (`'` or `"`) and returns a string token.

### Line 27
```cpp
Token readIdentifierOrKeyword();
```
Reads identifier-like text, then checks `keywords` to decide identifier vs keyword token.

### Line 28
```cpp
Token readOperator();
```
Reads operator or punctuation symbols (e.g., `+`, `==`, `(`, `)`) and returns matching token.

### Line 29
```cpp
};
```
Ends the `Lexer` class declaration.

## Concept summary

- Public API is intentionally small: construct lexer, call `tokenize()`.
- Private members hold scanning state (`src`, `pos`, `line`, `col`).
- Helper methods split lexing into clear stages: movement, skipping, then token-specific readers.
- `keywords` map supports fast keyword detection after reading identifier text.
