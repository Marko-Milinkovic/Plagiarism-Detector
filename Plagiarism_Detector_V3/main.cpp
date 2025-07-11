// main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <set>      // For storing unique fingerprints
#include <algorithm> // For std::set_intersection, std::set_union

// Include our custom classes
#include "Lexer.h"
#include "AST.h"
#include "Parser.h"

// --- Function to collect all subtree hashes from an AST ---
// IMPORTANT: Changed parameter from 'const ASTNodePtr& node' to 'const ASTNode* node'
// to correctly handle temporary unique_ptrs and avoid ownership issues during recursion.
void collectASTFingerprints(const ASTNode* node, std::set<unsigned long long>& fingerprints) {
    if (!node) {
        return;
    }

    // Special handling for ForStatementNode to canonicalize it to a WhileStatementNode structure
    if (auto for_node = dynamic_cast<const ForStatementNode*>(node)) { // Use const_cast for safety
        // A for loop is canonicalized to a sequence of statements:
        // initializer;
        // while (condition) {
        //   body;
        //   increment;
        // }

        // 1. Collect fingerprints from the initializer (if any). This is a statement before the loop.
        if (for_node->initializer) {
            // Check if the initializer is a VariableDeclarationNode
            if (for_node->initializer->kind == ASTNodeKind::VARIABLE_DECLARATION) {
                // If so, process it directly as a VariableDeclarationNode
                collectASTFingerprints(for_node->initializer.get(), fingerprints);
            } else {
                // Otherwise, it's an expression. Wrap it in a conceptual ExpressionStatementNode.
                auto initializer_stmt = std::make_unique<ExpressionStatementNode>(ASTNodePtr(for_node->initializer->clone()));
                collectASTFingerprints(initializer_stmt.get(), fingerprints);
            }
        }

        // 2. Create a temporary conceptual AST for the equivalent while loop
        auto canonical_while_body = std::make_unique<BlockStatementNode>();
        if (for_node->body) {
            if (auto block_ptr = dynamic_cast<const BlockStatementNode*>(for_node->body.get())) {
                for (const auto& stmt : block_ptr->statements) {
                    canonical_while_body->statements.push_back(ASTNodePtr(stmt->clone()));
                }
            } else {
                canonical_while_body->statements.push_back(ASTNodePtr(for_node->body->clone()));
            }
        }
        if (for_node->increment) {
            auto increment_stmt = std::make_unique<ExpressionStatementNode>(ASTNodePtr(for_node->increment->clone()));
            canonical_while_body->statements.push_back(std::move(increment_stmt));
        }

        auto canonical_while = std::make_unique<WhileStatementNode>(
            ASTNodePtr(for_node->condition->clone()),
            std::move(canonical_while_body));

        // 3. Collect fingerprints from the conceptual 'while' node.
        collectASTFingerprints(canonical_while.get(), fingerprints);

        return; // We have fully processed this ForStatementNode.
    }

    // For all other nodes, simply add their hash and recurse into children
    fingerprints.insert(node->getCanonicalHash());

    // Recursively collect hashes from children based on their specific types
    if (auto program_node = dynamic_cast<const ProgramNode*>(node)) {
        for (const auto& decl : program_node->declarations) {
            collectASTFingerprints(decl.get(), fingerprints);
        }
    }
    else if (auto func_def_node = dynamic_cast<const FunctionDefinitionNode*>(node)) {
        collectASTFingerprints(func_def_node->return_type.get(), fingerprints);
        collectASTFingerprints(func_def_node->identifier.get(), fingerprints);
        for (const auto& param : func_def_node->parameters) {
            collectASTFingerprints(param.get(), fingerprints);
        }
        collectASTFingerprints(func_def_node->body.get(), fingerprints);
    }
    else if (auto var_decl_node = dynamic_cast<const VariableDeclarationNode*>(node)) {
        collectASTFingerprints(var_decl_node->type.get(), fingerprints);
        collectASTFingerprints(var_decl_node->identifier.get(), fingerprints);
        if (var_decl_node->initializer) {
            collectASTFingerprints(var_decl_node->initializer.get(), fingerprints);
        }
    }
    else if (auto if_node = dynamic_cast<const IfStatementNode*>(node)) {
        collectASTFingerprints(if_node->condition.get(), fingerprints);
        collectASTFingerprints(if_node->then_branch.get(), fingerprints);
        if (if_node->else_branch) {
            collectASTFingerprints(if_node->else_branch.get(), fingerprints);
        }
    }
    else if (auto while_node = dynamic_cast<const WhileStatementNode*>(node)) {
        collectASTFingerprints(while_node->condition.get(), fingerprints);
        collectASTFingerprints(while_node->body.get(), fingerprints);
    }
    else if (auto return_node = dynamic_cast<const ReturnStatementNode*>(node)) {
        if (return_node->expression) {
            collectASTFingerprints(return_node->expression.get(), fingerprints);
        }
    }
    else if (auto expr_stmt_node = dynamic_cast<const ExpressionStatementNode*>(node)) {
        collectASTFingerprints(expr_stmt_node->expression.get(), fingerprints);
    }
    else if (auto block_node = dynamic_cast<const BlockStatementNode*>(node)) {
        for (const auto& stmt : block_node->statements) {
            collectASTFingerprints(stmt.get(), fingerprints);
        }
    }
    else if (auto binary_expr_node = dynamic_cast<const BinaryExpressionNode*>(node)) {
        collectASTFingerprints(binary_expr_node->left.get(), fingerprints);
        collectASTFingerprints(binary_expr_node->right.get(), fingerprints);
    }
    else if (auto unary_expr_node = dynamic_cast<const UnaryExpressionNode*>(node)) {
        collectASTFingerprints(unary_expr_node->operand.get(), fingerprints);
    }
    else if (auto func_call_node = dynamic_cast<const FunctionCallNode*>(node)) {
        collectASTFingerprints(func_call_node->callee.get(), fingerprints);
        for (const auto& arg : func_call_node->arguments) {
            collectASTFingerprints(arg.get(), fingerprints);
        }
    }
    else if (auto param_node = dynamic_cast<const ParameterNode*>(node)) {
        collectASTFingerprints(param_node->type.get(), fingerprints);
        collectASTFingerprints(param_node->identifier.get(), fingerprints);
    }
    // Base cases (IdentifierNode, NumberLiteralNode, StringLiteralNode, CharLiteralNode, TypeNode)
    // do not have children, so no recursive calls needed for them.
}


// --- Function to calculate Jaccard Similarity between two sets of fingerprints ---
double calculateJaccardSimilarity(const std::set<unsigned long long>& set1, const std::set<unsigned long long>& set2) {
    if (set1.empty() && set2.empty()) {
        return 100.0;
    }
    if (set1.empty() || set2.empty()) {
        return 0.0;
    }

    std::set<unsigned long long> intersection;
    std::set_intersection(set1.begin(), set1.end(),
        set2.begin(), set2.end(),
        std::inserter(intersection, intersection.begin()));

    std::set<unsigned long long> union_set;
    // THIS WAS THE BUG: std::set_union(set1.begin(), set2.end(), set2.begin(), set2.end(), ...)
    std::set_union(set1.begin(), set1.end(),
        set2.begin(), set2.end(),
        std::inserter(union_set, union_set.begin()));

    return static_cast<double>(intersection.size()) / union_set.size() * 100.0;
}

int main() {
    // Example C++ code snippets to test AST hashing and similarity
    std::string code_snippet_A = R"(
int calculate_sum(int x, int y) {
    int total = x + y;
    if (total > 10) {
        return total * 2;
    } else {
        return total;
    }
}
    )";

    // Code B: Plagiarized from A, with variable renaming and minor changes
    std::string code_snippet_B = R"(
int compute_sum(int a, int b) { // Function name changed, params renamed
    int result = a + b; // Variable renamed
    if (result > 10) {
        return result * 2;
    } else {
        return result;
    }
}
    )";

    // Code C: Structurally different
    std::string code_snippet_C = R"(
// Code Snippet C: using a for loop
int main() {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += i;
    }
    return 0;
}
    )";

    // Code D: Plagiarized from A, with variable renaming AND commutative operator reordering
    std::string code_snippet_D = R"(
int calculate_sum_reordered(int y_param, int x_param) { // Function name changed, params renamed
    int total_val = y_param + x_param; // Order of operands swapped (x + y -> y + x)
    if (10 < total_val) { // Order of operands swapped in comparison
        return 2 * total_val; // Order of operands swapped in multiplication
    } else {
        return total_val;
    }
}
    )";

    // NEW: Code E: Equivalent to Snippet C, but using a for loop
    std::string code_snippet_E = R"(
int main() {
    int sum = 0;
    int i = 0;
    while (i < 10) {
        sum += i;
        i++;
    }
    return 0;
}
)";


    std::cout << "--- AST Hashing and Similarity Test ---" << std::endl;

    std::vector<std::pair<std::string, std::string>> code_documents = {
        {"Snippet A", code_snippet_A},
        {"Snippet B", code_snippet_B},
        {"Snippet C", code_snippet_C},
        {"Snippet D", code_snippet_D},
        {"Snippet E", code_snippet_E}
    };

    std::vector<std::pair<std::string, std::set<unsigned long long>>> document_fingerprints;

    for (const auto& doc_pair : code_documents) {
        const std::string& doc_name = doc_pair.first;
        const std::string& doc_content = doc_pair.second;

        std::cout << "\nProcessing " << doc_name << "..." << std::endl;

        // 1. Lexical Analysis
        Lexer lexer(doc_content);
        std::vector<std::string> tokens = lexer.tokenize();
        std::cout << "  Tokenized: ";
        for (const auto& token : tokens) {
            std::cout << token << " ";
        }
        std::cout << std::endl;

        // 2. Parsing (Building AST)
        ASTNodePtr ast_root;
        try {
            Parser parser(tokens);
            ast_root = parser.parse();
            std::cout << "  AST built successfully." << std::endl;
            // std::cout << ast_root->toString() << std::endl; // Uncomment to see full AST
        }
        catch (const std::runtime_error& e) {
            std::cerr << "  ERROR: Parsing failed for " << doc_name << ": " << e.what() << std::endl;
            continue; // Skip to next document if parsing fails
        }

        // 3. Collect AST Fingerprints (Subtree Hashes)
        std::set<unsigned long long> fingerprints;
        if (ast_root) {
            collectASTFingerprints(ast_root.get(), fingerprints); // Pass raw pointer
        }
        document_fingerprints.push_back({ doc_name, fingerprints });
        std::cout << "  Collected " << fingerprints.size() << " unique AST fingerprints." << std::endl;
    }

    std::cout << "\n--- Pairwise AST Similarity Results ---" << std::endl;
    double similarity_threshold = 70.0; // Example threshold
    std::cout << "Threshold: " << similarity_threshold << "%" << std::endl << std::endl;

    for (size_t i = 0; i < document_fingerprints.size(); ++i) {
        for (size_t j = i + 1; j < document_fingerprints.size(); ++j) {
            const std::string& doc1_name = document_fingerprints[i].first;
            const std::set<unsigned long long>& fp1 = document_fingerprints[i].second;
            const std::string& doc2_name = document_fingerprints[j].first;
            const std::set<unsigned long long>& fp2 = document_fingerprints[j].second;

            double similarity = calculateJaccardSimilarity(fp1, fp2);

            std::cout << "Similarity between " << doc1_name << " and " << doc2_name << ": "
                << similarity << "%";

            if (similarity >= similarity_threshold) {
                std::cout << " (POTENTIAL PLAGIARISM DETECTED!)";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "---------------------------------------" << std::endl;

    return 0;
}