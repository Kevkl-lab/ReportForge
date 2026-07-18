#pragma once

#include <string>
#include <vector>
#include "../core/enums.h"

namespace reportforge::models {

/**
 * @brief Represents a vulnerability or finding discovered during a penetration test.
 */
struct Finding {
    int id{0};
    int projectId{0};                 // Parent project ID
    std::string title;
    core::Severity severity{core::Severity::Medium};
    double cvssScore{5.0};             // CVSS v3.1 Base Score
    std::string owaspCategory;         // e.g. A01:2021-Broken Access Control
    std::string cwe;                   // e.g. CWE-79
    std::string description;           // Supports Markdown
    std::string impact;                // Supports Markdown
    std::string recommendation;        // Supports Markdown
    std::string proofOfConcept;        // Supports Markdown (e.g. code snippets)
    std::string reproductionSteps;     // Supports Markdown
    std::string evidence;              // Text evidence details
    std::string affectedAssets;        // Affected URLs, IPs, hosts
    core::FindingStatus status{core::FindingStatus::Open};
    std::string tags;                  // Comma-separated list of tags
};

} // namespace reportforge::models
