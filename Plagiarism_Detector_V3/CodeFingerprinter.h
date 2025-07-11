// CodeFingerprinter.h
#ifndef PLAGIARISM_DETECTOR_CODE_FINGERPRINTER_H
#define PLAGIAGISM_DETECTOR_CODE_FINGERPRINTER_H

#include <string>
#include <vector>
#include <set>
#include <deque> // For the sliding window

// --- CodeFingerprinter Class ---
class CodeFingerprinter {
public:
    // Constructor
    // k_gram_size: length of k-grams
    // window_size: length of the sliding window (w)
    CodeFingerprinter(size_t k_gram_size, size_t window_size);

    // Generates fingerprints for a given token stream
    std::set<unsigned long long> generateFingerprints(const std::vector<std::string>& tokens);

private:
    size_t k; // k-gram size
    size_t w; // window size

    // Parameters for polynomial rolling hash
    const unsigned long long BASE = 31; // A prime number
    const unsigned long long MODULUS = 1e9 + 7; // A large prime modulus

    // Computes the polynomial rolling hash for a given k-gram
    unsigned long long computeHash(const std::vector<std::string>& k_gram) const;

    // Updates a rolling hash efficiently
    unsigned long long updateRollingHash(unsigned long long old_hash, const std::string& old_token,
        const std::string& new_token, unsigned long long power_of_base) const;
};

#endif // PLAGIAGISM_DETECTOR_CODE_FINGERPRINTER_H
