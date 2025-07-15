# C++ Code Plagiarism Detector

# Table of Contents

1. [Project Overview](#project-overview)
2. [Key Features](#key-features)
3. [Core Components and Technical Deep Dive](#core-components-and-technical-deep-dive)
   - [1. Lexical Analysis (Lexer)](#1-lexical-analysis-lexer)
   - [2. Syntactic Analysis (Parser & AST)](#2-syntactic-analysis-parser--ast)
   - [3. Plagiarism Detection (AST Subtree Hashing & Similarity Comparison)](#3-plagiarism-detection-ast-subtree-hashing--similarity-comparison)
4. [Example Usage and Detection Capabilities](#example-usage-and-detection-capabilities)
5. [How to Compile and Run](#how-to-compile-and-run)
6. [Future Enhancements](#future-enhancements)

## Project Overview

This project implements a robust C++ code plagiarism detection system. Unlike simple text-based comparisons, this detector operates on the Abstract Syntax Tree (AST) representation of code, making it resilient to common obfuscation techniques such as variable renaming, reordering of commutative operations, and structural transformations (e.g., converting for loops to while loops). By canonicalizing the AST and applying a sophisticated polynomial hashing scheme to its subtrees, the system can accurately identify structural similarities between code snippets.

## Key Features

- **AST-Based Analysis:** Goes beyond simple text comparison by analyzing the structural representation of code through Abstract Syntax Trees
- **Obfuscation Resilience:** Detects plagiarism even when code is modified through variable renaming, comment changes, or whitespace alterations
- **Commutative Operation Handling:** Recognizes that expressions like `a + b` and `b + a` are structurally equivalent
- **Loop Transformation Detection:** Identifies when `for` loops are converted to equivalent `while` loops to hide plagiarism
- **Polynomial Hashing:** Uses sophisticated hashing algorithms to create unique fingerprints for code subtrees
- **Jaccard Similarity Measurement:** Employs proven statistical methods to quantify code similarity with configurable thresholds
- **Comprehensive Tokenization:** Handles all major C++ language constructs including keywords, operators, literals, and preprocessor directives
- **Memory-Safe Implementation:** Uses modern C++ features like `std::unique_ptr` for automatic memory management
- **Normalization Pipeline:** Converts identifiers and literals to generic representations to focus on structural similarity
- **Scalable Architecture:** Designed with extensibility in mind for future language support and feature additions

## Core Components and Technical Deep Dive

The plagiarism detector is built upon three main phases: Lexical Analysis, Syntactic Analysis (Parsing & AST Construction), and Plagiarism Detection (AST Subtree Hashing & Similarity Comparison).

### 1. Lexical Analysis (Lexer)

**Files:** `Lexer.h`, `Lexer.cpp`

**Purpose:** The Lexer component is responsible for breaking down the raw C++ source code into a stream of meaningful tokens. This is the first step in transforming human-readable code into a machine-understandable format.

#### Data Structures & Algorithms:

**Token Types (TokenType enum):** Defines a comprehensive set of categories for C++ constructs, including `KEYWORD`, `IDENTIFIER`, `OPERATOR`, `DELIMITER`, `LITERAL_INT`, `LITERAL_FLOAT`, `LITERAL_STRING`, `LITERAL_CHAR`, `PREPROCESSOR`, `UNKNOWN`, and `END_OF_FILE`. This enumeration provides a standardized representation for each token.

**Keyword & Operator Sets (`std::set<std::string>`, `std::map<std::string, TokenType>`):**
- `keywords`: A `std::set` stores all C++ keywords for efficient O(log N) lookup (or O(1) average for unordered_set).
- `single_char_operators_delimiters`: A `std::set<char>` for quick checking of single-character operators and delimiters.
- `multi_char_operators`: A `std::map` is used to store multi-character operators (e.g., `==`, `++`, `>>`) and their corresponding TokenType, enabling the lexer to prioritize longest possible matches (e.g., `>>` over `>`).

**State Management:** `current_pos` (size_t) tracks the current position in the input source code, facilitating sequential scanning.

**Tokenization Logic:** The `tokenize()` method iteratively skips whitespace, handles single-line (`//`) and multi-line (`/* ... */`) comments, and preprocessor directives (`#include`, `#define`, etc.). It then identifies and extracts identifiers, keywords, numeric literals (integers and basic floats), string literals, character literals, and operators/delimiters.

**Normalization:** A crucial aspect for plagiarism detection is normalization. The Lexer normalizes `IDENTIFIER` tokens, `NUMBER_LITERAL` tokens, `STRING_LITERAL` tokens, and `CHAR_LITERAL` tokens to generic string values (e.g., "IDENTIFIER", "NUMBER_LITERAL"). This ensures that plagiarism is detected based on structural similarity rather than exact variable names or literal values. For example, `int x = 5;` and `int y = 10;` will both tokenize `IDENTIFIER` and `NUMBER_LITERAL`, allowing the parser to build similar AST structures.

### 2. Syntactic Analysis (Parser & AST)

**Files:** `AST.h`, `AST.cpp`, `Parser.h`, `Parser.cpp`, `custom_grammar.txt`

**Purpose:** The Parser takes the token stream from the Lexer and builds an Abstract Syntax Tree (AST). The AST is a hierarchical representation of the program's structure, abstracting away syntactic details and focusing on the logical relationships between code elements. This abstraction is fundamental for robust plagiarism detection.

#### Data Structures & Algorithms:

**Abstract Syntax Tree (AST):**
- **Base Class (ASTNode):** A polymorphic base class (`ASTNode`) defines the common interface for all nodes in the AST, including `kind` (an `ASTNodeKind` enum), `toString()` for debugging, `getCanonicalHash()` for structural hashing, and `clone()` for deep copying.
- **Node Hierarchy:** Specific node classes (e.g., `ProgramNode`, `FunctionDefinitionNode`, `BinaryExpressionNode`, `IfStatementNode`, `WhileStatementNode`, `ForStatementNode`, `IdentifierNode`, `NumberLiteralNode`) inherit from `ASTNode`. Each node stores pointers (managed by `std::unique_ptr` for automatic memory management) to its child nodes, forming the tree structure.
- **ASTNodePtr:** A `std::unique_ptr<ASTNode>` alias simplifies memory management and ownership transfer of AST nodes.

**Recursive Descent Parser:** The Parser implements a recursive descent parsing strategy. Each grammar rule (defined in `custom_grammar.txt`) typically corresponds to a parsing method (e.g., `parseProgram()`, `parseFunctionDefinition()`, `parseExpression()`).

**Grammar (custom_grammar.txt):** Defines a simplified C-like grammar using EBNF (Extended Backus-Naur Form). This grammar guides the parser's construction.

**Operator Precedence Parsing:** The expression parsing methods (`parseAssignmentExpression`, `parseLogicalOrExpression`, etc.) are structured to correctly handle operator precedence and associativity, building the AST accurately according to C++ rules.

**Canonicalization within AST (getCanonicalHash()):** This is a critical component for plagiarism detection. Each `ASTNode` implements `getCanonicalHash()`, which computes a hash value for the subtree rooted at that node. The canonicalization rules applied here are:

- **Identifier Normalization:** As handled by the Lexer, all identifiers are treated generically (e.g., `x`, `y`, `temp` all become "IDENTIFIER" tokens), so their specific names do not affect the AST hash.
- **Literal Normalization:** Similarly, numeric, string, and character literals are normalized to generic representations (e.g., `5`, `100`, `"hello"` all become "NUMBER_LITERAL", "STRING_LITERAL"), preventing changes in values from affecting structural similarity.
- **Commutative Operator Handling:** For commutative operators (`+`, `*`, `==`, `!=`, `&&`, `||`, `&`, `|`, `^`), the `getCanonicalHash()` method for `BinaryExpressionNode` sorts the hashes of its left and right operands before combining them. This ensures that `a + b` and `b + a` produce the same canonical hash.
- **Relational Operator Swapping:** For relational operators (`<`, `>`, `<=`, `>=`), if the left operand's canonical hash is greater than the right operand's, the operands are conceptually swapped, and the operator is flipped (e.g., `a < b` becomes `b > a` if `hash(a) > hash(b)`). This canonicalizes expressions like `x < 10` and `10 > x`.

**Polynomial Hashing (combine_hashes):** The `combine_hashes` function (defined in `AST.h`) uses a polynomial hashing scheme with two prime bases (`AST_HASH_BASE_1`, `AST_HASH_BASE_2`) and a large prime modulus (`AST_HASH_MODULUS`) to combine the hashes of child nodes. This recursive combination ensures that the hash of a parent node is a function of its children's hashes and its own kind, effectively creating a unique fingerprint for each subtree. This helps in minimizing hash collisions and ensures that the hash reflects the structural properties of the subtree.

**For Loop Canonicalization (in main.cpp collectASTFingerprints):** A for loop is conceptually transformed into an equivalent while loop structure (initializer; while (condition) { body; increment; }) before its fingerprints are collected. This handles cases where a student might convert a for loop to a while loop to obscure plagiarism.

### 3. Plagiarism Detection (AST Subtree Hashing & Similarity Comparison)

**Files:** `main.cpp`, `AST.h`, `AST.cpp`

**Purpose:** This component leverages the canonical AST hashes to identify structural similarities between code snippets. Instead of relying on external fingerprinting algorithms, the system directly collects and compares all canonical subtree hashes generated during the AST traversal. This approach provides a fine-grained comparison of code structure.

#### Data Structures & Algorithms:

**AST Subtree Hash Collection (collectASTFingerprints in main.cpp):**
This is a recursive depth-first traversal function that iterates through the entire Abstract Syntax Tree.

For each node encountered in the AST, it calls the node's `getCanonicalHash()` method. This method, as described above, computes a unique polynomial hash representing the canonical form of the subtree rooted at that node, incorporating the hashes of its children and its own type.

Each computed canonical hash is then inserted into a `std::set<unsigned long long>`. The `std::set` is crucial here:
- **Uniqueness:** It automatically stores only unique hash values, effectively creating a "fingerprint set" for the entire code snippet. Duplicate subtrees (e.g., `x + y` appearing multiple times) will only contribute one hash to the set.
- **Efficient Lookup:** `std::set` provides logarithmic time complexity for insertion and lookup, which is efficient for building the fingerprint set.

Special handling is included for `ForStatementNode` where it's conceptually transformed into an equivalent `WhileStatementNode` structure before its subtrees are hashed. This ensures that syntactically different but semantically equivalent control flow structures yield similar fingerprints.

**Jaccard Similarity:**

**Concept:** The Jaccard Similarity (also known as the Jaccard Index or Intersection over Union) is a statistic used for gauging the similarity and diversity of sample sets. In this context, it measures the overlap between two sets of AST subtree hashes.

**Formula:** Given two sets of fingerprints, A and B, the Jaccard Similarity J(A,B) is calculated as the size of their intersection divided by the size of their union:

$$J(A, B) = \frac{|A \cap B|}{|A \cup B|}$$

**Implementation (calculateJaccardSimilarity in main.cpp):**
- `std::set_intersection`: This standard library algorithm efficiently computes the intersection of `set1` and `set2`, populating a new `std::set` with common hash values.
- `std::set_union`: This algorithm computes the union of `set1` and `set2`, populating another `std::set` with all unique hash values present in either set.
- The sizes of the resulting intersection and union sets are then used in the Jaccard formula.

**Interpretation:** The result is a floating-point number between 0.0 and 1.0, which is then typically scaled to a percentage (0 to 100). A value closer to 100 indicates a high degree of structural similarity between the two code snippets, strongly suggesting plagiarism. A configurable `similarity_threshold` (e.g., 70) is used to automatically flag pairs that exceed this level as potential plagiarism. This threshold can be adjusted based on desired sensitivity.

## Example Usage and Detection Capabilities

The following examples demonstrate the detector's ability to identify plagiarism across various obfuscation techniques:

### Original Code (Snippet A)
```cpp
int calculate_sum(int x, int y) {
    int total = x + y;
    if (total > 10) {
        return total * 2;
    } else {
        return total;
    }
}
```

### Plagiarized Code with Variable Renaming (Snippet B)
```cpp
int compute_sum(int a, int b) { // Function name changed, params renamed
    int result = a + b; // Variable renamed
    if (result > 10) {
        return result * 2;
    } else {
        return result;
    }
}
```
**Detection Result:** ~100% similarity - Successfully detects plagiarism despite variable renaming

### Plagiarized Code with Commutative Reordering (Snippet D)
```cpp
int calculate_sum_reordered(int y_param, int x_param) {
    int total_val = y_param + x_param; // Order swapped: x + y -> y + x
    if (10 < total_val) { // Comparison flipped: total > 10 -> 10 < total
        return 2 * total_val; // Multiplication reordered: total * 2 -> 2 * total
    } else {
        return total_val;
    }
}
```
**Detection Result:** ~100% similarity - Successfully detects plagiarism despite operand reordering

### Loop Structure Transformation
**Original For Loop (Snippet C):**
```cpp
int main() {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += i;
    }
    return 0;
}
```

**Converted While Loop (Snippet E):**
```cpp
int main() {
    int sum = 0;
    int i = 0;
    while (i < 10) {
        sum += i;
        i++;
    }
    return 0;
}
```
**Detection Result:** ~75-85% similarity - Successfully detects structural equivalence between for and while loops

### Completely Different Code
```cpp
void print_hello() {
    std::cout << "Hello, World!" << std::endl;
}
```
**Detection Result:** ~0-10% similarity - Correctly identifies structurally different code

### Key Detection Strengths Demonstrated

1. **Variable Renaming Immunity:** The detector normalizes all identifiers to generic tokens, making it immune to simple variable name changes.

2. **Commutative Operation Handling:** Mathematical and logical operations are canonicalized so that `a + b` and `b + a` produce identical fingerprints.

3. **Control Flow Transformation:** For loops are internally converted to equivalent while loop structures for comparison.

4. **Structural Focus:** The system focuses on code structure rather than surface-level syntax, making it robust against formatting changes and minor modifications.

5. **Configurable Sensitivity:** The similarity threshold can be adjusted based on the desired level of strictness for plagiarism detection.

## How to Compile and Run

### Prerequisites:
- A C++ compiler (e.g., g++).
- make (optional, but recommended for larger projects).

### Compilation:

1. Navigate to the root directory of the project.
2. Compile the source files:

```bash
g++ -std=c++17 -O2 -Wall -I. AST.cpp Lexer.cpp Parser.cpp CodeFingerprinter.cpp main.cpp -o plagiarism_detector
```

- `-std=c++17`: Ensures C++17 features (like `std::unique_ptr` and `std::set` operations) are available.
- `-O2`: Optimization flag.
- `-Wall`: Enables all warnings (good practice for robust code).
- `-I.`: Includes the current directory for header files.
- `-o plagiarism_detector`: Specifies the output executable name.

### Running:

```bash
./plagiarism_detector
```

The `main.cpp` contains example code snippets (A, B, C, D, E) and demonstrates their processing through the Lexer, Parser, AST construction, subtree hash collection, and pairwise similarity calculation. The output will show the tokenized form, AST build status, number of collected AST subtree hashes, and the Jaccard Similarity percentage for each pair, highlighting potential plagiarism.


## Future Enhancements

- **Error Reporting:** Enhance error messages from Lexer and Parser to provide more specific line and column numbers for easier debugging.

- **Advanced Canonicalization:** Implement more sophisticated AST canonicalization rules, such as:
  - Normalizing control flow (e.g., if-else if-else chains).
  - Handling loop unrolling/re-rolling.
  - Canonicalizing switch statements.

- **Plagiarism Visualization:** Develop a graphical interface to visualize the ASTs and highlight the common subtrees or differences between plagiarized code snippets.

- **Scalability:** Investigate techniques for handling very large codebases, potentially involving parallel processing of files or database storage for fingerprints.

- **Language Support:** Extend the Lexer and Parser to support other programming languages (e.g., Java, Python).

- **Excluding Boilerplate:** Implement mechanisms to exclude common boilerplate code or library includes from analysis to reduce noise.

This project serves as a strong foundation for understanding and applying core computer science concepts to a practical and challenging problem.
