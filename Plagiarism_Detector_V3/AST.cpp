// AST.cpp
#include "AST.h" // Include the header where they are declared extern

// Define the global constants
const unsigned long long AST_HASH_BASE_1 = 31;
const unsigned long long AST_HASH_BASE_2 = 37;
const unsigned long long AST_HASH_MODULUS = 1000000007ULL;

// Define the global sets
const std::set<std::string> commutative_operators = {
    "+", "*", "==", "!=", "&&", "||", "&", "|", "^"
};

const std::set<std::string> relational_operators_for_canonicalization = {
    "<", ">", "<=", ">="
};