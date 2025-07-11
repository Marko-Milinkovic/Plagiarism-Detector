// Lexer.cpp
#include "Lexer.h" // Include our header
#include <iostream>
#include <sstream>

// Constructor definition
Lexer::Lexer(const std::string& source_code) : source(source_code), current_pos(0) {}

std::vector<std::string> Lexer::tokenize() {
    std::vector<std::string> tokens;
    while (current_pos < source.length()) {
        skipWhitespace();

        if (current_pos >= source.length()) { break; }

        if (handleComments()) {
            continue;
        }
        if (handlePreprocessor()) { // Fixed typo: hadnlePreprocessor -> handlePreprocessor
            continue;
        }

        std::string token_value = "";
        TokenType type = TokenType::UNKNOWN; // Initialized to UNKNOWN

        if (isAlpha(source[current_pos])) { // keywords or identifiers
            token_value = readIdentifierOrKeyword();
            type = getIdentifierOrKeywordType(token_value);
            // Normalize identifiers
            if (type == TokenType::IDENTIFIER) {
                token_value = "IDENTIFIER";
            }
        }
        else if (isDigit(source[current_pos])) { // numeric literals
            token_value = readNumber();
            // Determine if it's int or float here if needed
            type = TokenType::LITERAL_INT; // Default to int
            token_value = "NUMBER_LITERAL"; // Normalize value
        }
        else if (source[current_pos] == '"') { // string literals
            token_value = readStringLiteral();
            type = TokenType::LITERAL_STRING;
            token_value = "STRING_LITERAL"; // Normalize value
        }
        else if (source[current_pos] == '\'') { // char literals
            token_value = readCharLiteral();
            type = TokenType::LITERAL_CHAR;
            token_value = "CHAR_LITERAL"; // Normalize value
        }
        else if (isSingleCharOperatorDelimiter(source[current_pos])) { // operators or delimiters
            token_value = readOperatorOrDelimiter();
            type = getOperatorOrDelimiterType(token_value); // Will use the new map for type if multi-char
        }
        else {
            // unknown character, move past it and potentially report an error
            // For now, just advance and continue
            current_pos++;
            continue;
        }

        if (!token_value.empty()) {
            tokens.push_back(token_value);
        }
    }
    return tokens;
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::isSingleCharOperatorDelimiter(char c) const {
    return single_char_operators_delimiters.count(c);
}

void Lexer::skipWhitespace() {
    while (current_pos < source.length() && isWhitespace(source[current_pos])) {
        current_pos++;
    }
}

bool Lexer::handleComments() {
    if (current_pos + 1 < source.length()) {
        // Handle single-line comments
        if (source[current_pos] == '/' && source[current_pos + 1] == '/') {
            while (current_pos < source.length() && source[current_pos] != '\n') {
                current_pos++;
            }
            // Move past newline if it exists, otherwise it's end of file
            if (current_pos < source.length() && source[current_pos] == '\n') {
                current_pos++;
            }
            return true;
        }
        // Handle multi-line comments
        if (source[current_pos] == '/' && source[current_pos + 1] == '*') {
            current_pos += 2; // Move past "/*"
            while (current_pos + 1 < source.length() && !(source[current_pos] == '*' && source[current_pos + 1] == '/')) {
                current_pos++;
            }
            if (current_pos + 1 < source.length()) { // Found "*/"
                current_pos += 2; // Move past "*/"
            }
            else {
                // Unclosed multi-line comment, consume till end of file
                current_pos = source.length();
            }
            return true;
        }
    }
    return false;
}

bool Lexer::handlePreprocessor() { // Fixed typo: hadnlePreprocessor -> handlePreprocessor
    if (current_pos < source.length() && source[current_pos] == '#') {
        while (current_pos < source.length() && source[current_pos] != '\n') {
            current_pos++;
        }
        // Move past newline if it exists, otherwise it's end of file
        if (current_pos < source.length() && source[current_pos] == '\n') {
            current_pos++;
        }
        return true;
    }
    return false;
}

std::string Lexer::readIdentifierOrKeyword() {
    size_t start = current_pos;
    while (current_pos < source.length() && isAlphaNumeric(source[current_pos])) {
        current_pos++;
    }
    return source.substr(start, current_pos - start);
}

std::string Lexer::readNumber() {
    size_t start = current_pos;
    while (current_pos < source.length() && isDigit(source[current_pos])) {
        current_pos++;
    }
    // Basic support for floating point (e.g., 123.45)
    if (current_pos < source.length() && source[current_pos] == '.') {
        current_pos++;
        while (current_pos < source.length() && isDigit(source[current_pos])) {
            current_pos++;
        }
    }
    // TODO: Add support for exponents (e.g., 1e-5), suffixes (L, F, U)
    return source.substr(start, current_pos - start);
}

std::string Lexer::readStringLiteral() {
    size_t start = current_pos;
    current_pos++; // Move past opening quote '"'
    while (current_pos < source.length() && source[current_pos] != '"') {
        // Handle escaped quotes within string
        if (source[current_pos] == '\\' && current_pos + 1 < source.length()) {
            current_pos++; // Skip escaped character
        }
        current_pos++;
    }
    if (current_pos < source.length() && source[current_pos] == '"') {
        current_pos++; // Move past closing quote '"'
    }
    // Return the literal including quotes; it will be normalized to STRING_LITERAL
    return source.substr(start, current_pos - start);
}

std::string Lexer::readCharLiteral() {
    size_t start = current_pos;
    current_pos++; // Move past opening quote '''
    while (current_pos < source.length() && source[current_pos] != '\'') {
        // Handle escaped characters within char literal (e.g., '\n', '\'')
        if (source[current_pos] == '\\' && current_pos + 1 < source.length()) {
            current_pos++; // Skip escaped character
        }
        current_pos++;
    }
    if (current_pos < source.length() && source[current_pos] == '\'') {
        current_pos++; // Move past closing quote '''
    }
    // Return the literal including quotes; it will be normalized to CHAR_LITERAL
    return source.substr(start, current_pos - start);
}


std::string Lexer::readOperatorOrDelimiter() {
    size_t start = current_pos;
    current_pos++; // Start with the first character of the potential operator/delimiter

    // Try to match longest possible multi-character operator first
    std::string two_char_op = "";
    if (current_pos < source.length()) {
        two_char_op = source.substr(start, 2);
        if (multi_char_operators.count(two_char_op)) {
            current_pos++; // Consume the second character
            return two_char_op;
        }
    }
    // If no two-character operator matches, or if only one character left,
    // revert to the single character
    return source.substr(start, 1);
}

TokenType Lexer::getIdentifierOrKeywordType(const std::string& value) const {
    // Keywords are stored in lowercase for case-insensitive comparison
    // But C++ keywords are case-sensitive. The `keywords` set already uses lowercase.
    // If the input `value` is mixed case, convert it to lowercase for lookup.
    // However, C++ identifiers are case-sensitive, so we might want to store actual keyword case.
    // For now, assuming `keywords` set contains lowercase versions and `value` comes as-is.
    // If you want case-insensitive keyword match (not typical for C++ but useful for normalization),
    // you'd transform `value` to lowercase here.
    if (keywords.count(value)) {
        return TokenType::KEYWORD;
    }
    return TokenType::IDENTIFIER;
}

TokenType Lexer::getOperatorOrDelimiterType(const std::string& value) const {
    if (multi_char_operators.count(value)) {
        return TokenType::OPERATOR;
    }
    // For single-character operators/delimiters, we just return OPERATOR/DELIMITER
    // For more fine-grained type detection, you'd need another map for single chars.
    // For plagiarism, differentiating `+` from `*` is enough.
    return TokenType::OPERATOR; // Default for single chars
}

