// Parser.h
#ifndef PLAGIARISM_DETECTOR_PARSER_H
#define PLAGIARISM_DETECTOR_PARSER_H

#include "Lexer.h"  // To get the token types and the lexer's output
#include "AST.h"    // To define the AST nodes that the parser will build
#include <string>
#include <vector>
#include <stdexcept> // For parsing errors

class Parser {
public:
    // Constructor: takes the token stream from the Lexer
    Parser(std::vector<std::string> tokens)
        : tokens(std::move(tokens)), current_pos(0) {}

    // The main public method that starts the parsing process
    ASTNodePtr parse();

private:
    std::vector<std::string> tokens;
    size_t current_pos;

    // --- Token handling helpers ---
    // Look at the current token without consuming it
    std::string peek() const;
    // Consume the current token and advance to the next
    void consume();
    // Consume the current token only if it matches the expected value
    void match(const std::string& expected_value);
    // Check if the current token matches the expected value
    bool check(const std::string& expected_value) const;
    // Check if the current token is a specific TokenType
    bool checkType(TokenType expected_type) const;
    // Get the TokenType for a given token string
    TokenType getTokenType(const std::string& token_value) const;

    // --- Recursive Descent Parsing Methods (one for each grammar rule) ---
    ASTNodePtr parseProgram();
    ASTNodePtr parseFunctionDefinition();
    ASTNodePtr parseParameter();
    ASTNodePtr parseType();
    ASTNodePtr parseStatement();
    ASTNodePtr parseDeclarationStatement();
    ASTNodePtr parseIfStatement();
    ASTNodePtr parseWhileStatement();
    ASTNodePtr parseForStatement(); // NEW DECLARATION
    ASTNodePtr parseReturnStatement();
    ASTNodePtr parseExpressionStatement();
    ASTNodePtr parseBlockStatement();

    // Expression parsing with operator precedence (from lowest to highest)
    ASTNodePtr parseExpression();
    ASTNodePtr parseAssignmentExpression();
    ASTNodePtr parseConditionalExpression();
    ASTNodePtr parseLogicalOrExpression();
    ASTNodePtr parseLogicalAndExpression();
    ASTNodePtr parseEqualityExpression();
    ASTNodePtr parseRelationalExpression();
    ASTNodePtr parseShiftExpression();
    ASTNodePtr parseAdditiveExpression();
    ASTNodePtr parseMultiplicativeExpression();
    ASTNodePtr parseUnaryExpression();
    ASTNodePtr parsePostfixExpression();
    ASTNodePtr parsePrimaryExpression();
};

#endif // PLAGIAGISM_DETECTOR_PARSER_H // Note: This should be PLAGIARISM_DETECTOR_PARSER_H
