#pragma once

#include <string>
#include "../core/enums.h"

namespace reportforge::models {

/**
 * @brief Represents a reusable finding template.
 */
struct FindingTemplate {
    int id{0};
    std::string title;
    core::Severity severity{core::Severity::Medium};
    double cvssScore{5.0};
    std::string owaspCategory;
    std::string cwe;
    std::string description;
    std::string impact;
    std::string recommendation;
};

} // namespace reportforge::models
