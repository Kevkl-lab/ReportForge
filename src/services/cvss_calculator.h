#pragma once

#include <string>

namespace reportforge::services {

/**
 * @brief Utility class to calculate CVSS v3.1 base score from metric indices.
 */
class CvssCalculator {
public:
    struct Metrics {
        int attackVector{0};        // 0: Network (AV:N), 1: Adjacent (AV:A), 2: Local (AV:L), 3: Physical (AV:P)
        int attackComplexity{0};    // 0: Low (AC:L), 1: High (AC:H)
        int privilegesRequired{0};  // 0: None (PR:N), 1: Low (PR:L), 2: High (PR:H)
        int userInteraction{0};     // 0: None (UI:N), 1: Required (UI:R)
        int scope{0};               // 0: Unchanged (S:U), 1: Changed (S:C)
        int confidentiality{0};     // 0: High (C:H), 1: Low (C:L), 2: None (C:N)
        int integrity{0};           // 0: High (I:H), 1: Low (I:L), 2: None (I:N)
        int availability{0};        // 0: High (A:H), 1: Low (A:L), 2: None (A:N)
    };

    /**
     * @brief Computes the CVSS v3.1 Base Score based on the official specification.
     * @return Double value between 0.0 and 10.0.
     */
    static double calculate(const Metrics& metrics);

    /**
     * @brief Constructs the official CVSS v3.1 Vector String.
     */
    static std::string toStringVector(const Metrics& metrics);
};

} // namespace reportforge::services
