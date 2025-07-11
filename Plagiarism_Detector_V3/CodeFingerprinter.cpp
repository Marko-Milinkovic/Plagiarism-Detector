// CodeFingerprinter.cpp
#include "CodeFingerprinter.h"
#include <iostream>
#include <limits> // For std::numeric_limits

// Constructor
CodeFingerprinter::CodeFingerprinter(size_t k_gram_size, size_t window_size)
    : k(k_gram_size), w(window_size) {
    if (k == 0 || w == 0) {
        // Handle error: k_gram_size and window_size must be positive
        std::cerr << "Error: k_gram_size and window_size must be greater than 0." << std::endl;
        // You might throw an exception or handle this more gracefully in a real application
    }
}

// Simple hash function for a string (can be improved for better distribution)
// For this example, we'll use a basic sum of character values.
// In a real rolling hash, we'd map strings to integers for more efficient arithmetic.
// For now, we'll just use std::hash for simplicity in computeHash,
// but for rolling hash, we'd need a custom string-to-int mapping.
unsigned long long CodeFingerprinter::computeHash(const std::vector<std::string>& k_gram) const {
    unsigned long long current_hash = 0;
    unsigned long long p_power = 1; // Power of BASE

    for (const std::string& token : k_gram) {
        // Map each token string to an integer for hashing.
        // For simplicity, we'll use std::hash<std::string> but this is not ideal for rolling hash.
        // A better approach for rolling hash would be to map each unique token to a unique small integer ID.
        unsigned long long token_val = std::hash<std::string>{}(token);
        current_hash = (current_hash + (token_val * p_power) % MODULUS) % MODULUS;
        p_power = (p_power * BASE) % MODULUS;
    }
    return current_hash;
}

// This function is designed for a true rolling hash, which requires token values to be integers.
// For the current `computeHash` using `std::hash<std::string>`, this rolling update is not directly applicable
// without mapping string tokens to integer IDs first.
// We will simplify the Winnowing to recompute hash for each k-gram for now,
// or introduce a token-to-ID mapping if performance becomes an issue.
unsigned long long CodeFingerprinter::updateRollingHash(unsigned long long old_hash, const std::string& old_token,
    const std::string& new_token, unsigned long long power_of_base) const {
    // This is a placeholder for a true rolling hash.
    // It requires string tokens to be mapped to integer IDs to work efficiently.
    // For now, we will recompute hashes for each k-gram.
    // Implementing a full string-to-int mapping and then rolling hash is more complex.
    return 0; // Not used in current simplified approach
}


std::set<unsigned long long> CodeFingerprinter::generateFingerprints(const std::vector<std::string>& tokens) {
    std::set<unsigned long long> fingerprints;

    if (tokens.size() < k) {
        // Not enough tokens to form a single k-gram
        return fingerprints;
    }

    // Step 1: Generate all k-gram hashes
    std::vector<unsigned long long> k_gram_hashes;
    for (size_t i = 0; i <= tokens.size() - k; ++i) {
        std::vector<std::string> k_gram(tokens.begin() + i, tokens.begin() + i + k);
        k_gram_hashes.push_back(computeHash(k_gram));
    }

    if (k_gram_hashes.empty()) {
        return fingerprints;
    }

    // Step 2: Apply Winnowing
    std::deque<std::pair<unsigned long long, size_t>> window; // Stores {hash, index} pairs
    unsigned long long min_hash_in_window = std::numeric_limits<unsigned long long>::max();
    size_t min_hash_index = 0;
    unsigned long long last_fingerprint_hash = 0; // To avoid duplicate fingerprints from consecutive windows

    for (size_t i = 0; i < k_gram_hashes.size(); ++i) {
        unsigned long long current_hash = k_gram_hashes[i];

        // Remove hashes from the front of the window that are no longer in the current window
        while (!window.empty() && window.front().second <= i - w) {
            window.pop_front();
        }

        // Maintain the window in increasing order of hashes (and increasing index for ties)
        while (!window.empty() && window.back().first >= current_hash) {
            window.pop_back();
        }
        window.push_back({ current_hash, i });

        // The minimum hash in the current window is always at the front of the deque
        min_hash_in_window = window.front().first;
        min_hash_index = window.front().second;

        // Add fingerprint if it's the rightmost minimum in the current window
        // and it's different from the last added fingerprint (to avoid redundant additions)
        // The original Winnowing algorithm adds the rightmost minimum.
        // We add it if it's the minimum for the current window and its index is the rightmost minimum.
        // A simpler rule for adding to the fingerprint set is to add the minimum hash
        // whenever the minimum hash *changes* or when the window *slides past* the previous minimum.
        // For simplicity and to ensure some density, we'll add the current minimum
        // if it's the rightmost minimum in the window and it hasn't been added yet from this window.

        // A common simplification for Winnowing implementation:
        // Add the minimum hash to the fingerprint set whenever the window reaches its full size (i.e., i >= w-1)
        // and the minimum hash is the rightmost minimum in that window.
        // Or, more simply, add the rightmost minimum from the window.
        // Let's use a simplified approach for adding to fingerprints, which is to add the minimum
        // whenever it's the rightmost minimum in the current window, and it's not a duplicate of the *last*
        // added fingerprint (to avoid consecutive identical fingerprints).

        // Simplified rule for adding fingerprints:
        // Add the minimum hash if it's the rightmost minimum in the current window
        // and it's different from the *last* fingerprint added.
        if (min_hash_in_window != last_fingerprint_hash) {
            // Check if the current minimum is the rightmost in the window
            // This requires checking elements *after* the current minimum in the original k_gram_hashes
            // For simplicity, we'll just add the front of the deque if it's a new hash.
            // A more precise Winnowing implementation tracks the rightmost minimum.
            // For now, let's add the minimum if it's new.
            if (fingerprints.find(min_hash_in_window) == fingerprints.end()) {
                fingerprints.insert(min_hash_in_window);
                last_fingerprint_hash = min_hash_in_window;
            }
        }
    }
    return fingerprints;
}