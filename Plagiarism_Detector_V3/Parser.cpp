// Parser.cpp
#include "Parser.h"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

// --- Helper Functions for Parser ---

// Get the TokenType for a given token string.
// This is a simplified version; in a real scenario, the lexer would provide this.
TokenType Parser::getTokenType(const std::string& token_value) const {
    static const std::unordered_map<std::string, TokenType> token_map = {
        // Keywords
        {"if", TokenType::KEYWORD}, {"else", TokenType::KEYWORD}, {"while", TokenType::KEYWORD},
        {"for", TokenType::KEYWORD}, // NEW: Added for keyword
        {"return", TokenType::KEYWORD}, {"int", TokenType::KEYWORD}, {"void", TokenType::KEYWORD},
        {"bool", TokenType::KEYWORD}, {"true", TokenType::KEYWORD}, {"false", TokenType::KEYWORD},
        {"nullptr", TokenType::KEYWORD}, {"const", TokenType::KEYWORD},

        // Operators
        {"=", TokenType::OPERATOR}, {"+=", TokenType::OPERATOR}, {"-=", TokenType::OPERATOR},
        {"*=", TokenType::OPERATOR}, {"/=", TokenType::OPERATOR}, {"%=", TokenType::OPERATOR},
        {"==", TokenType::OPERATOR}, {"!=", TokenType::OPERATOR}, {"<", TokenType::OPERATOR},
        {">", TokenType::OPERATOR}, {"<=", TokenType::OPERATOR}, {">=", TokenType::OPERATOR},
        {"&&", TokenType::OPERATOR}, {"||", TokenType::OPERATOR}, {"!", TokenType::OPERATOR},
        {"++", TokenType::OPERATOR}, {"--", TokenType::OPERATOR}, {"+", TokenType::OPERATOR},
        {"-", TokenType::OPERATOR}, {"*", TokenType::OPERATOR}, {"/", TokenType::OPERATOR},
        {"%", TokenType::OPERATOR}, {"->", TokenType::OPERATOR}, {"::", TokenType::OPERATOR},
        {"<<", TokenType::OPERATOR}, {">>", TokenType::OPERATOR},

        // Delimiters
        {"(", TokenType::DELIMITER}, {")", TokenType::DELIMITER}, {"{", TokenType::DELIMITER},
        {"}", TokenType::DELIMITER}, {";", TokenType::DELIMITER}, {",", TokenType::DELIMITER},
        {"?", TokenType::DELIMITER}, {":", TokenType::DELIMITER}
    };

    auto it = token_map.find(token_value);
    if (it != token_map.end()) {
        return it->second;
    }

    if (token_value == "IDENTIFIER") return TokenType::IDENTIFIER;
    if (token_value == "NUMBER_LITERAL") return TokenType::LITERAL_INT;
    if (token_value == "STRING_LITERAL") return TokenType::LITERAL_STRING;
    if (token_value == "CHAR_LITERAL") return TokenType::LITERAL_CHAR;
    if (token_value == "END_OF_FILE") return TokenType::END_OF_FILE;

    return TokenType::UNKNOWN;
}

// Look at the current token without consuming it
std::string Parser::peek() const {
    if (current_pos < tokens.size()) {
        return tokens[current_pos];
    }
    return "END_OF_FILE";
}

// Consume the current token and advance to the next
void Parser::consume() {
    if (current_pos < tokens.size()) {
        current_pos++;
    }
}

// Consume the current token only if it matches the expected value
void Parser::match(const std::string& expected_value) {
    if (peek() == expected_value) {
        consume();
    }
    else {
        throw std::runtime_error("Syntax Error: Expected '" + expected_value + "' but found '" + peek() + "' at token index " + std::to_string(current_pos));
    }
}

// Check if the current token matches the expected value
bool Parser::check(const std::string& expected_value) const {
    return peek() == expected_value;
}

// Check if the current token is a specific TokenType
bool Parser::checkType(TokenType expected_type) const {
    return getTokenType(peek()) == expected_type;
}

// --- Main Parsing Logic ---

// The public entry point for parsing
ASTNodePtr Parser::parse() {
    return parseProgram();
}

// parseProgram: Parses the top-level program
ASTNodePtr Parser::parseProgram() {
    auto program_node = std::make_unique<ProgramNode>();
    while (peek() != "END_OF_FILE") {
        size_t original_pos = current_pos;

        ASTNodePtr temp_type = parseType();

        if (!checkType(TokenType::IDENTIFIER)) {
            current_pos = original_pos;
            throw std::runtime_error("Syntax Error: Expected IDENTIFIER after type at top level, found '" + peek() + "' at token index " + std::to_string(current_pos));
        }
        consume();

        std::string token_after_identifier = peek();

        current_pos = original_pos;

        if (token_after_identifier == "(") {
            program_node->declarations.push_back(parseFunctionDefinition());
        }
        else if (token_after_identifier == "=" || token_after_identifier == ";" || token_after_identifier == ",") {
            program_node->declarations.push_back(parseDeclarationStatement());
        }
        else {
            throw std::runtime_error("Syntax Error: Unexpected token after identifier at top level: '" + token_after_identifier + "' at token index " + std::to_string(current_pos));
        }
    }
    return program_node;
}

// parseFunctionDefinition: Parses a function definition
ASTNodePtr Parser::parseFunctionDefinition() {
    auto return_type = parseType();
    auto identifier_name = peek();
    match("IDENTIFIER");
    auto identifier = std::make_unique<IdentifierNode>(identifier_name);

    match("(");
    std::vector<ASTNodePtr> parameters;
    if (!check(")")) {
        parameters.push_back(parseParameter());
        while (check(",")) {
            consume();
            parameters.push_back(parseParameter());
        }
    }
    match(")");

    auto body = parseBlockStatement();

    auto func_node = std::make_unique<FunctionDefinitionNode>(std::move(return_type), std::move(identifier), std::move(body));
    func_node->parameters = std::move(parameters);
    return func_node;
}

// parseParameter: Parses a function parameter
ASTNodePtr Parser::parseParameter() {
    auto type = parseType();
    auto identifier_name = peek();
    match("IDENTIFIER");
    auto identifier = std::make_unique<IdentifierNode>(identifier_name);
    return std::make_unique<ParameterNode>(std::move(type), std::move(identifier));
}

// parseType: Parses a type (e.g., int, void, IDENTIFIER, std::string, std::cout for qualified names)
ASTNodePtr Parser::parseType() {
    std::string type_name_str = "";
    if (check("const")) {
        type_name_str += peek();
        consume();
        type_name_str += " ";
    }

    if (checkType(TokenType::KEYWORD) || checkType(TokenType::IDENTIFIER)) {
        type_name_str += peek();
        consume();
        while (check("::")) {
            match("::");
            type_name_str += "::" + peek();
            match("IDENTIFIER");
        }
        return std::make_unique<TypeNode>(type_name_str);
    }
    throw std::runtime_error("Syntax Error: Expected a type but found '" + peek() + "' at token index " + std::to_string(current_pos));
}

// parseStatement: Parses any kind of statement
ASTNodePtr Parser::parseStatement() {
    if (check("{")) {
        return parseBlockStatement();
    }
    if (check("if")) {
        return parseIfStatement();
    }
    if (check("while")) {
        return parseWhileStatement();
    }
    if (check("for")) { // NEW: Handle for loop
        return parseForStatement();
    }
    if (check("return")) {
        return parseReturnStatement();
    }

    size_t original_pos = current_pos;

    std::string current_token = peek();
    if (getTokenType(current_token) == TokenType::KEYWORD || getTokenType(current_token) == TokenType::IDENTIFIER) {
        consume();
        if (current_token == "const") {
            if (getTokenType(peek()) == TokenType::KEYWORD || getTokenType(peek()) == TokenType::IDENTIFIER) {
                consume();
            }
        }

        if (checkType(TokenType::IDENTIFIER)) {
            current_pos = original_pos;
            return parseDeclarationStatement();
        }
        current_pos = original_pos;
    }

    return parseExpressionStatement();
}

// parseDeclarationStatement: Parses a variable declaration
ASTNodePtr Parser::parseDeclarationStatement() {
    auto type = parseType();
    auto identifier_name = peek();
    match("IDENTIFIER");
    auto identifier = std::make_unique<IdentifierNode>(identifier_name);

    ASTNodePtr initializer = nullptr;
    if (check("=")) {
        consume();
        initializer = parseExpression();
    }

    match(";");
    return std::make_unique<VariableDeclarationNode>(std::move(type), std::move(identifier), std::move(initializer));
}

// parseIfStatement: Parses an if statement
ASTNodePtr Parser::parseIfStatement() {
    match("if");
    match("(");
    auto condition = parseExpression();
    match(")");
    auto then_branch = parseStatement();

    ASTNodePtr else_branch = nullptr;
    if (check("else")) {
        consume();
        else_branch = parseStatement();
    }

    return std::make_unique<IfStatementNode>(std::move(condition), std::move(then_branch), std::move(else_branch));
}

// parseWhileStatement: Parses a while loop
ASTNodePtr Parser::parseWhileStatement() {
    match("while");
    match("(");
    auto condition = parseExpression();
    match(")");
    auto body = parseStatement();
    return std::make_unique<WhileStatementNode>(std::move(condition), std::move(body));
}

// NEW: parseForStatement()
ASTNodePtr Parser::parseForStatement() {
    match("for");
    match("(");

    ASTNodePtr initializer = nullptr;
    if (!check(";")) {
        // A for loop initializer can be a declaration or an expression statement
        // We'll simplify and assume it's either a declaration or an expression statement
        // For a more robust parser, you'd need to distinguish more carefully.
        size_t original_pos = current_pos;
        std::string current_token = peek();
        if (getTokenType(current_token) == TokenType::KEYWORD || getTokenType(current_token) == TokenType::IDENTIFIER) {
            // Peek ahead to see if it looks like a declaration
            consume();
            if (current_token == "const") {
                if (getTokenType(peek()) == TokenType::KEYWORD || getTokenType(peek()) == TokenType::IDENTIFIER) {
                    consume();
                }
            }
            if (checkType(TokenType::IDENTIFIER)) { // If it's a type followed by an identifier, it's a declaration
                current_pos = original_pos;
                initializer = parseDeclarationStatement(); // This will consume the trailing ';'
            }
            else {
                current_pos = original_pos;
                initializer = parseExpressionStatement(); // This will consume the trailing ';'
            }
        }
        else {
            initializer = parseExpressionStatement(); // This will consume the trailing ';'
        }
    }
    else {
        match(";"); // Consume the semicolon if initializer is empty
    }

    ASTNodePtr condition = nullptr;
    if (!check(";")) {
        condition = parseExpression();
    }
    match(";");

    ASTNodePtr increment = nullptr;
    if (!check(")")) {
        increment = parseExpression();
    }
    match(")");

    auto body = parseStatement();

    return std::make_unique<ForStatementNode>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}


// parseReturnStatement: Parses a return statement
ASTNodePtr Parser::parseReturnStatement() {
    match("return");
    ASTNodePtr expr = nullptr;
    if (!check(";")) {
        expr = parseExpression();
    }
    match(";");
    return std::make_unique<ReturnStatementNode>(std::move(expr));
}

// parseExpressionStatement: Parses an expression followed by a semicolon
ASTNodePtr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    match(";");
    return std::make_unique<ExpressionStatementNode>(std::move(expr));
}

// parseBlockStatement: Parses a block of statements { ... }
ASTNodePtr Parser::parseBlockStatement() {
    auto block = std::make_unique<BlockStatementNode>();
    match("{");
    while (!check("}")) {
        if (check("END_OF_FILE")) {
            throw std::runtime_error("Syntax Error: Expected '}' but found end of file.");
        }
        block->statements.push_back(parseStatement());
    }
    match("}");
    return block;
}

// parseExpression: Entry point for parsing an expression
ASTNodePtr Parser::parseExpression() {
    return parseAssignmentExpression();
}

ASTNodePtr Parser::parseAssignmentExpression() {
    auto expr = parseConditionalExpression();
    if (check("=") || check("+=") || check("-=") || check("*=") || check("/=")) {
        std::string op = peek();
        consume();
        auto right = parseAssignmentExpression();
        return std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseConditionalExpression() {
    return parseLogicalOrExpression();
}

ASTNodePtr Parser::parseLogicalOrExpression() {
    auto expr = parseLogicalAndExpression();
    while (check("||")) {
        std::string op = peek();
        consume();
        auto right = parseLogicalAndExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseLogicalAndExpression() {
    auto expr = parseEqualityExpression();
    while (check("&&")) {
        std::string op = peek();
        consume();
        auto right = parseEqualityExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseEqualityExpression() {
    auto expr = parseRelationalExpression();
    while (check("==") || check("!=")) {
        std::string op = peek();
        consume();
        auto right = parseRelationalExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseRelationalExpression() {
    auto expr = parseShiftExpression();
    while (check("<") || check(">") || check("<=") || check(">=")) {
        std::string op = peek();
        consume();
        auto right = parseShiftExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseShiftExpression() {
    auto expr = parseAdditiveExpression();
    while (check("<<") || check(">>")) {
        std::string op = peek();
        consume();
        auto right = parseAdditiveExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseAdditiveExpression() {
    auto expr = parseMultiplicativeExpression();
    while (check("+") || check("-")) {
        std::string op = peek();
        consume();
        auto right = parseMultiplicativeExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseMultiplicativeExpression() {
    auto expr = parseUnaryExpression();
    while (check("*") || check("/") || check("%")) {
        std::string op = peek();
        consume();
        auto right = parseUnaryExpression();
        expr = std::make_unique<BinaryExpressionNode>(std::move(expr), std::move(right), op);
    }
    return expr;
}

ASTNodePtr Parser::parseUnaryExpression() {
    if (check("++") || check("--") || check("+") || check("-") || check("!")) {
        std::string op = peek();
        consume();
        auto operand = parseUnaryExpression();
        return std::make_unique<UnaryExpressionNode>(std::move(operand), op);
    }
    return parsePostfixExpression();
}

ASTNodePtr Parser::parsePostfixExpression() {
    auto expr = parsePrimaryExpression();

    while (check("++") || check("--") || check("(")) {
        if (check("++") || check("--")) {
            std::string op = peek();
            consume();
            expr = std::make_unique<UnaryExpressionNode>(std::move(expr), op);
        }
        else if (check("(")) {
            auto call_node = std::make_unique<FunctionCallNode>(std::move(expr));
            match("(");
            if (!check(")")) {
                call_node->arguments.push_back(parseExpression());
                while (check(",")) {
                    consume();
                    call_node->arguments.push_back(parseExpression());
                }
            }
            match(")");
            expr = std::move(call_node);
        }
    }
    return expr;
}

ASTNodePtr Parser::parsePrimaryExpression() {
    std::string token_val = peek();

    if (check("(")) {
        consume();
        auto expr = parseExpression();
        match(")");
        return expr;
    }

    if (checkType(TokenType::IDENTIFIER)) {
        consume();
        std::string full_identifier_name = token_val;
        while (check("::")) {
            match("::");
            full_identifier_name += "::" + peek();
            match("IDENTIFIER");
        }
        return std::make_unique<IdentifierNode>(full_identifier_name);
    }

    if (checkType(TokenType::LITERAL_INT)) {
        consume();
        return std::make_unique<NumberLiteralNode>(token_val);
    }
    if (checkType(TokenType::LITERAL_STRING)) {
        consume();
        return std::make_unique<StringLiteralNode>(token_val);
    }
    if (checkType(TokenType::LITERAL_CHAR)) {
        consume();
        return std::make_unique<CharLiteralNode>(token_val);
    }

    throw std::runtime_error("Syntax Error: Unexpected token in primary expression: '" + token_val + "' at token index " + std::to_string(current_pos));
}
