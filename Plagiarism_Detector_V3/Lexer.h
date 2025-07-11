// Lexer.h
#ifndef PLAGIARISM_DETECTOR_LEXER_H
#define PLAGIARISM_DETECTOR_LEXER_H // <-- CORRECTED: PLAGIARISM_DETECTOR_LEXER_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm> // For std::transform if used for token normalization

// --- Token Types ---
enum class TokenType {
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    DELIMITER,
    LITERAL_INT,
    LITERAL_FLOAT, // Added for completeness, if readNumber supports it fully
    LITERAL_STRING,
    LITERAL_CHAR, // Added
    PREPROCESSOR,
    UNKNOWN,
    END_OF_FILE // Added for parser's convenience
};

// --- Lexer Class ---
class Lexer {
public:
    // Constructor takes the source code string
    Lexer(const std::string& source_code);

    // Main function to tokenize the source code
    std::vector<std::string> tokenize();

private:
    std::string source;
    size_t current_pos;

    // Sets of C++ components (made const to ensure they are initialized once)
    const std::set<std::string> keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel",
        "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor",
        "bool", "break", "case", "catch", "char", "char8_t", "char16_t",
        "char32_t", "class", "compl", "concept", "const", "consteval",
        "constexpr", "constinit", "const_cast", "continue", "co_await",
        "co_return", "co_yield", "decltype", "default", "delete", "do",
        "double", "dynamic_cast", "else", "enum", "explicit", "export",
        "extern", "false", "float", "for", "friend", "goto", "if",
        "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
        "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
        "protected", "public", "reflexpr", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static",
        "static_assert", "static_cast", "struct", "switch", "synchronized",
        "template", "this", "thread_local", "throw", "true", "try",
        "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };

    // Single character operators and delimiters
    const std::set<char> single_char_operators_delimiters = {
        '+', '-', '*', '/', '%', '=', '<', '>', '!', '&', '|', '^', '~',
        '(', ')', '{', '}', '[', ']', ';', ',', '.', ':', '?'
    };

    // Multi-character operators
    // Using a map to quickly map string to TokenType (which will be OPERATOR)
    const std::map<std::string, TokenType> multi_char_operators = {
        {"==", TokenType::OPERATOR}, {"!=", TokenType::OPERATOR}, {"<=", TokenType::OPERATOR},
        {">=", TokenType::OPERATOR}, {"&&", TokenType::OPERATOR}, {"||", TokenType::OPERATOR},
        {"++", TokenType::OPERATOR}, {"--", TokenType::OPERATOR}, {"<<", TokenType::OPERATOR},
        {">>", TokenType::OPERATOR}, {"->", TokenType::OPERATOR}, {"::", TokenType::OPERATOR},
        {"+=", TokenType::OPERATOR}, {"-=", TokenType::OPERATOR}, {"*=", TokenType::OPERATOR},
        {"/=", TokenType::OPERATOR}, {"%=", TokenType::OPERATOR}, {"&=", TokenType::OPERATOR},
        {"|=", TokenType::OPERATOR}, {"^=", TokenType::OPERATOR}, {"<<=", TokenType::OPERATOR},
        {">>=", TokenType::OPERATOR}
    };


    // Helper functions for character classification (made const)
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isWhitespace(char c) const;
    bool isSingleCharOperatorDelimiter(char c) const; // Renamed from isOperatorDelimiter

    // Core lexing functions
    void skipWhitespace();
    bool handleComments();
    bool handlePreprocessor(); // Fixed typo here
    std::string readIdentifierOrKeyword();
    std::string readNumber();
    std::string readStringLiteral();
    std::string readCharLiteral(); // Added
    std::string readOperatorOrDelimiter();

    // Type classification (made const)
    TokenType getIdentifierOrKeywordType(const std::string& value) const;
    TokenType getOperatorOrDelimiterType(const std::string& value) const;
};

#endif // PLAGIARISM_DETECTOR_LEXER_H
