#pragma once
#include <string>

enum class TokenType {
    // Literals
    NUMBER, STRING, BOOL_TRUE, BOOL_FALSE, NIL,

    // Identifiers & Keywords
    IDENTIFIER,
    LET, CONST, FN, RETURN, IF, ELSE, ELIF,
    WHILE, FOR, IN, BREAK, CONTINUE,
    PRINT, INPUT,
    IMPORT,
    // Cybersecurity reserved keywords (future)
    SCAN, PAYLOAD, ENCRYPT, DECRYPT, HASH,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT, POWER,
    EQ, NEQ, LT, GT, LTE, GTE,
    AND, OR, NOT,
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN,
    BIT_AND, BIT_OR, BIT_XOR, BIT_NOT, LSHIFT, RSHIFT,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, SEMICOLON, COLON, DOT, ARROW,
    NEWLINE,

    // Special
    EOF_TOKEN, UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;

    Token(TokenType t, std::string v, int ln, int c)
        : type(t), value(std::move(v)), line(ln), col(c) {}

    std::string toString() const;
};
