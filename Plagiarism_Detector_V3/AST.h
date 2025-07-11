// AST.h
#ifndef PLAGIARISM_DETECTOR_AST_H
#define PLAGIARISM_DETECTOR_AST_H

#include <string>
#include <vector>
#include <memory> // For std::unique_ptr
#include <map>    // For optional use in nodes, e.g., function parameters
#include <algorithm> // For std::sort
#include <numeric>   // For std::accumulate (not directly used but good to have)
#include <functional> // For std::hash (not directly used but good to have)
#include <set> // For checking commutative and relational operators

#include "Lexer.h" // To get TokenType definition

// --- Hashing Constants (declared as extern) ---
extern const unsigned long long AST_HASH_BASE_1;
extern const unsigned long long AST_HASH_BASE_2;
extern const unsigned long long AST_HASH_MODULUS;

// Helper functions (inline is crucial here to prevent redefinition errors for functions)
inline unsigned long long combine_hashes(unsigned long long h1, unsigned long long h2) {
    return (h1 * AST_HASH_BASE_1 + h2) % AST_HASH_MODULUS;
}

inline unsigned long long hash_string(const std::string& s) {
    unsigned long long h = 0;
    unsigned long long p_power = 1;
    for (char c : s) {
        h = (h + (static_cast<unsigned long long>(c) * p_power) % AST_HASH_MODULUS) % AST_HASH_MODULUS;
        p_power = (p_power * AST_HASH_BASE_2) % AST_HASH_MODULUS; // Use a different base for string hashing
    }
    return h;
}

// --- Set of Commutative Operators (declared as extern) ---
extern const std::set<std::string> commutative_operators;

// --- Set of Relational Operators that can be canonicalized by swapping operands (declared as extern) ---
extern const std::set<std::string> relational_operators_for_canonicalization;


// --- AST Node Kinds ---
enum class ASTNodeKind {
    PROGRAM,
    FUNCTION_DEFINITION,
    VARIABLE_DECLARATION,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION_STATEMENT,
    BLOCK_STATEMENT,
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    FUNCTION_CALL,
    IDENTIFIER,
    NUMBER_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    PARAMETER,
    TYPE
};

// --- Base AST Node Class ---
class ASTNode {
public:
    ASTNodeKind kind;

    ASTNode(ASTNodeKind k) : kind(k) {}
    virtual ~ASTNode() = default;

    virtual std::string toString(int indent = 0) const {
        std::string s = "";
        for (int i = 0; i < indent; ++i) s += "  ";
        s += "Node Kind: " + getKindString(kind) + "\n";
        return s;
    }

    virtual unsigned long long getCanonicalHash() const = 0;
    virtual ASTNode* clone() const = 0;

    static std::string getKindString(ASTNodeKind k) {
        switch (k) {
        case ASTNodeKind::PROGRAM: return "PROGRAM";
        case ASTNodeKind::FUNCTION_DEFINITION: return "FUNCTION_DEFINITION";
        case ASTNodeKind::VARIABLE_DECLARATION: return "VARIABLE_DECLARATION";
        case ASTNodeKind::IF_STATEMENT: return "IF_STATEMENT";
        case ASTNodeKind::WHILE_STATEMENT: return "WHILE_STATEMENT";
        case ASTNodeKind::FOR_STATEMENT: return "FOR_STATEMENT";
        case ASTNodeKind::RETURN_STATEMENT: return "RETURN_STATEMENT";
        case ASTNodeKind::EXPRESSION_STATEMENT: return "EXPRESSION_STATEMENT";
        case ASTNodeKind::BLOCK_STATEMENT: return "BLOCK_STATEMENT";
        case ASTNodeKind::BINARY_EXPRESSION: return "BINARY_EXPRESSION";
        case ASTNodeKind::UNARY_EXPRESSION: return "UNARY_EXPRESSION";
        case ASTNodeKind::FUNCTION_CALL: return "FUNCTION_CALL";
        case ASTNodeKind::IDENTIFIER: return "IDENTIFIER";
        case ASTNodeKind::NUMBER_LITERAL: return "NUMBER_LITERAL";
        case ASTNodeKind::STRING_LITERAL: return "STRING_LITERAL";
        case ASTNodeKind::CHAR_LITERAL: return "CHAR_LITERAL";
        case ASTNodeKind::PARAMETER: return "PARAMETER";
        case ASTNodeKind::TYPE: return "TYPE";
        default: return "UNKNOWN_AST_NODE";
        }
    }
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

// --- Specific AST Node Classes (with getCanonicalHash and clone implementations) ---
// Classes are ordered to resolve dependencies (e.g., ExpressionStatementNode before ForStatementNode)

class TypeNode : public ASTNode {
public:
    std::string name;
    TypeNode(const std::string& n) : ASTNode(ASTNodeKind::TYPE), name(n) {}
    std::string toString(int indent = 0) const override {
        std::string s = ""; for (int i = 0; i < indent; ++i) s += "  "; s += "Type: " + name + "\n"; return s;
    }
    unsigned long long getCanonicalHash() const override {
        return combine_hashes(static_cast<unsigned long long>(kind), hash_string(name));
    }
    ASTNode* clone() const override { return new TypeNode(name); }
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(const std::string& n) : ASTNode(ASTNodeKind::IDENTIFIER), name(n) {}
    std::string toString(int indent = 0) const override {
        std::string s = ""; for (int i = 0; i < indent; ++i) s += "  "; s += "Identifier: " + name + "\n"; return s;
    }
    unsigned long long getCanonicalHash() const override { return static_cast<unsigned long long>(kind); }
    ASTNode* clone() const override { return new IdentifierNode(name); }
};

class NumberLiteralNode : public ASTNode {
public:
    std::string value;
    NumberLiteralNode(const std::string& val) : ASTNode(ASTNodeKind::NUMBER_LITERAL), value(val) {}
    std::string toString(int indent = 0) const override {
        std::string s = ""; for (int i = 0; i < indent; ++i) s += "  "; s += "Number Literal: " + value + "\n"; return s;
    }
    unsigned long long getCanonicalHash() const override { return static_cast<unsigned long long>(kind); }
    ASTNode* clone() const override { return new NumberLiteralNode(value); }
};

class StringLiteralNode : public ASTNode {
public:
    std::string value;
    StringLiteralNode(const std::string& val) : ASTNode(ASTNodeKind::STRING_LITERAL), value(val) {}
    std::string toString(int indent = 0) const override {
        std::string s = ""; for (int i = 0; i < indent; ++i) s += "  "; s += "String Literal: \"" + value + "\"\n"; return s;
    }
    unsigned long long getCanonicalHash() const override { return static_cast<unsigned long long>(kind); }
    ASTNode* clone() const override { return new StringLiteralNode(value); }
};

class CharLiteralNode : public ASTNode {
public:
    std::string value;
    CharLiteralNode(const std::string& val) : ASTNode(ASTNodeKind::CHAR_LITERAL), value(val) {}
    std::string toString(int indent = 0) const override {
        std::string s = ""; for (int i = 0; i < indent; ++i) s += "  "; s += "Char Literal: '" + value + "'\n"; return s;
    }
    unsigned long long getCanonicalHash() const override { return static_cast<unsigned long long>(kind); }
    ASTNode* clone() const override { return new CharLiteralNode(value); }
};

class BinaryExpressionNode : public ASTNode {
public:
    ASTNodePtr left;
    ASTNodePtr right;
    std::string op;
    BinaryExpressionNode(ASTNodePtr l, ASTNodePtr r, const std::string& o)
        : ASTNode(ASTNodeKind::BINARY_EXPRESSION), left(std::move(l)), right(std::move(r)), op(o) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); for (int i = 0; i < indent + 1; ++i) s += "  ";
        s += "Operator: " + op + "\n"; s += left->toString(indent + 1); s += right->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        unsigned long long current_left_hash = left->getCanonicalHash();
        unsigned long long current_right_hash = right->getCanonicalHash();
        std::string effective_op = op;
        bool children_should_be_sorted = false;
        if (commutative_operators.count(op)) { children_should_be_sorted = true; }
        else if (relational_operators_for_canonicalization.count(op)) {
            children_should_be_sorted = true;
            if (current_left_hash > current_right_hash) {
                std::swap(current_left_hash, current_right_hash);
                if (op == "<") effective_op = ">"; else if (op == ">") effective_op = "<";
                else if (op == "<=") effective_op = ">="; else if (op == ">=") effective_op = "<=";
            }
        }
        h = combine_hashes(h, hash_string(effective_op));
        if (children_should_be_sorted) {
            std::vector<unsigned long long> child_hashes;
            child_hashes.push_back(current_left_hash); child_hashes.push_back(current_right_hash);
            std::sort(child_hashes.begin(), child_hashes.end());
            h = combine_hashes(h, child_hashes[0]); h = combine_hashes(h, child_hashes[1]);
        }
        else {
            h = combine_hashes(h, current_left_hash); h = combine_hashes(h, current_right_hash);
        } return h;
    }
    ASTNode* clone() const override { return new BinaryExpressionNode(ASTNodePtr(left->clone()), ASTNodePtr(right->clone()), op); }
};

class UnaryExpressionNode : public ASTNode {
public:
    ASTNodePtr operand;
    std::string op;
    UnaryExpressionNode(ASTNodePtr o, const std::string& op_str)
        : ASTNode(ASTNodeKind::UNARY_EXPRESSION), operand(std::move(o)), op(op_str) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); for (int i = 0; i < indent + 1; ++i) s += "  ";
        s += "Operator: " + op + "\n"; s += operand->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, hash_string(op)); h = combine_hashes(h, operand->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override { return new UnaryExpressionNode(ASTNodePtr(operand->clone()), op); }
};

class FunctionCallNode : public ASTNode {
public:
    ASTNodePtr callee;
    std::vector<ASTNodePtr> arguments;
    FunctionCallNode(ASTNodePtr c) : ASTNode(ASTNodeKind::FUNCTION_CALL), callee(std::move(c)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += callee->toString(indent + 1);
        for (const auto& arg : arguments) { s += arg->toString(indent + 2); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, callee->getCanonicalHash());
        for (const auto& arg : arguments) { h = combine_hashes(h, arg->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override {
        auto new_node = new FunctionCallNode(ASTNodePtr(callee->clone()));
        for (const auto& arg : arguments) { new_node->arguments.push_back(ASTNodePtr(arg->clone())); } return new_node;
    }
};

class ParameterNode : public ASTNode {
public:
    ASTNodePtr type;
    ASTNodePtr identifier;
    ParameterNode(ASTNodePtr t, ASTNodePtr id)
        : ASTNode(ASTNodeKind::PARAMETER), type(std::move(t)), identifier(std::move(id)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += type->toString(indent + 1);
        s += identifier->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, type->getCanonicalHash()); h = combine_hashes(h, identifier->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override { return new ParameterNode(ASTNodePtr(type->clone()), ASTNodePtr(identifier->clone())); }
};

class ExpressionStatementNode : public ASTNode {
public:
    ASTNodePtr expression;
    ExpressionStatementNode(ASTNodePtr expr)
        : ASTNode(ASTNodeKind::EXPRESSION_STATEMENT), expression(std::move(expr)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += expression->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, expression->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override { return new ExpressionStatementNode(ASTNodePtr(expression->clone())); }
};

class BlockStatementNode : public ASTNode {
public:
    std::vector<ASTNodePtr> statements;
    BlockStatementNode() : ASTNode(ASTNodeKind::BLOCK_STATEMENT) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent);
        for (const auto& stmt : statements) { s += stmt->toString(indent + 1); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        for (const auto& stmt : statements) { h = combine_hashes(h, stmt->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override {
        auto new_node = new BlockStatementNode();
        for (const auto& stmt : statements) { new_node->statements.push_back(ASTNodePtr(stmt->clone())); } return new_node;
    }
};

class WhileStatementNode : public ASTNode {
public:
    ASTNodePtr condition;
    ASTNodePtr body;
    WhileStatementNode(ASTNodePtr cond, ASTNodePtr b)
        : ASTNode(ASTNodeKind::WHILE_STATEMENT), condition(std::move(cond)), body(std::move(b)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += condition->toString(indent + 1);
        s += body->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, condition->getCanonicalHash()); h = combine_hashes(h, body->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override { return new WhileStatementNode(ASTNodePtr(condition->clone()), ASTNodePtr(body->clone())); }
};

class ForStatementNode : public ASTNode {
public:
    ASTNodePtr initializer;
    ASTNodePtr condition;
    ASTNodePtr increment;
    ASTNodePtr body;
    ForStatementNode(ASTNodePtr init, ASTNodePtr cond, ASTNodePtr incr, ASTNodePtr b)
        : ASTNode(ASTNodeKind::FOR_STATEMENT),
        initializer(std::move(init)), condition(std::move(cond)),
        increment(std::move(incr)), body(std::move(b)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent);
        for (int i = 0; i < indent + 1; ++i) s += "  "; s += "Initializer:\n";
        if (initializer) s += initializer->toString(indent + 2); else s += "    (none)\n";
        for (int i = 0; i < indent + 1; ++i) s += "  "; s += "Condition:\n";
        if (condition) s += condition->toString(indent + 2); else s += "    (none)\n";
        for (int i = 0; i < indent + 1; ++i) s += "  "; s += "Increment:\n";
        if (increment) s += increment->toString(indent + 2); else s += "    (none)\n";
        for (int i = 0; i < indent + 1; ++i) s += "  "; s += "Body:\n";
        s += body->toString(indent + 2); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        if (initializer) { h = combine_hashes(h, initializer->getCanonicalHash()); }
        if (condition) { h = combine_hashes(h, condition->getCanonicalHash()); }
        if (increment) { h = combine_hashes(h, increment->getCanonicalHash()); }
        h = combine_hashes(h, body->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override {
        return new ForStatementNode(
            initializer ? ASTNodePtr(initializer->clone()) : nullptr,
            condition ? ASTNodePtr(condition->clone()) : nullptr,
            increment ? ASTNodePtr(increment->clone()) : nullptr,
            ASTNodePtr(body->clone())
        );
    }
};

class ReturnStatementNode : public ASTNode {
public:
    ASTNodePtr expression;
    ReturnStatementNode(ASTNodePtr expr = nullptr)
        : ASTNode(ASTNodeKind::RETURN_STATEMENT), expression(std::move(expr)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent);
        if (expression) { s += expression->toString(indent + 1); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        if (expression) { h = combine_hashes(h, expression->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override { return new ReturnStatementNode(expression ? ASTNodePtr(expression->clone()) : nullptr); }
};

class VariableDeclarationNode : public ASTNode {
public:
    ASTNodePtr type;
    ASTNodePtr identifier;
    ASTNodePtr initializer;
    VariableDeclarationNode(ASTNodePtr t, ASTNodePtr id, ASTNodePtr init = nullptr)
        : ASTNode(ASTNodeKind::VARIABLE_DECLARATION), type(std::move(t)), identifier(std::move(id)), initializer(std::move(init)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += type->toString(indent + 1);
        s += identifier->toString(indent + 1);
        if (initializer) { s += initializer->toString(indent + 1); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, type->getCanonicalHash()); h = combine_hashes(h, identifier->getCanonicalHash());
        if (initializer) { h = combine_hashes(h, initializer->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override {
        return new VariableDeclarationNode(
            ASTNodePtr(type->clone()), ASTNodePtr(identifier->clone()),
            initializer ? ASTNodePtr(initializer->clone()) : nullptr);
    }
};

class IfStatementNode : public ASTNode {
public:
    ASTNodePtr condition;
    ASTNodePtr then_branch;
    ASTNodePtr else_branch;
    IfStatementNode(ASTNodePtr cond, ASTNodePtr then_b, ASTNodePtr else_b = nullptr)
        : ASTNode(ASTNodeKind::IF_STATEMENT), condition(std::move(cond)),
        then_branch(std::move(then_b)), else_branch(std::move(else_b)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += condition->toString(indent + 1);
        s += then_branch->toString(indent + 1);
        if (else_branch) { s += else_branch->toString(indent + 1); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, condition->getCanonicalHash()); h = combine_hashes(h, then_branch->getCanonicalHash());
        if (else_branch) { h = combine_hashes(h, else_branch->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override {
        return new IfStatementNode(
            ASTNodePtr(condition->clone()), ASTNodePtr(then_branch->clone()),
            else_branch ? ASTNodePtr(else_branch->clone()) : nullptr);
    }
};

class FunctionDefinitionNode : public ASTNode {
public:
    ASTNodePtr return_type;
    ASTNodePtr identifier;
    std::vector<ASTNodePtr> parameters;
    ASTNodePtr body;
    FunctionDefinitionNode(ASTNodePtr ret_type, ASTNodePtr id, ASTNodePtr b)
        : ASTNode(ASTNodeKind::FUNCTION_DEFINITION), return_type(std::move(ret_type)),
        identifier(std::move(id)), body(std::move(b)) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent); s += return_type->toString(indent + 1);
        s += identifier->toString(indent + 1);
        for (const auto& param : parameters) { s += param->toString(indent + 2); }
        s += body->toString(indent + 1); return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        h = combine_hashes(h, return_type->getCanonicalHash()); h = combine_hashes(h, identifier->getCanonicalHash());
        for (const auto& param : parameters) { h = combine_hashes(h, param->getCanonicalHash()); }
        h = combine_hashes(h, body->getCanonicalHash()); return h;
    }
    ASTNode* clone() const override {
        auto new_node = new FunctionDefinitionNode(
            ASTNodePtr(return_type->clone()), ASTNodePtr(identifier->clone()), ASTNodePtr(body->clone()));
        for (const auto& param : parameters) { new_node->parameters.push_back(ASTNodePtr(param->clone())); } return new_node;
    }
};

class ProgramNode : public ASTNode {
public:
    std::vector<ASTNodePtr> declarations;
    ProgramNode() : ASTNode(ASTNodeKind::PROGRAM) {}
    std::string toString(int indent = 0) const override {
        std::string s = ASTNode::toString(indent);
        for (const auto& decl : declarations) { s += decl->toString(indent + 1); } return s;
    }
    unsigned long long getCanonicalHash() const override {
        unsigned long long h = static_cast<unsigned long long>(kind);
        for (const auto& decl : declarations) { h = combine_hashes(h, decl->getCanonicalHash()); } return h;
    }
    ASTNode* clone() const override {
        auto new_node = new ProgramNode();
        for (const auto& decl : declarations) { new_node->declarations.push_back(ASTNodePtr(decl->clone())); } return new_node;
    }
};

#endif // PLAGIARISM_DETECTOR_AST_H

