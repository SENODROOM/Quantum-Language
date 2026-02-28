#pragma once
#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer
{
public:
    explicit Lexer(const std::string &source);
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos;
    int line, col;

    static const std::unordered_map<std::string, TokenType> keywords;

    char current() const;
    char peek(int offset = 1) const;
    char advance();
    void skipWhitespace();
    void skipComment();

    Token readNumber();
    Token readString(char quote);
    Token readIdentifierOrKeyword();
    Token readOperator();
};
