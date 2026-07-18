#pragma once

#include <string>
#include <vector>
#include "../core/enums.h"

namespace reportforge::models {

/**
 * @brief Represents a penetration testing project.
 */
struct Project {
    int id{0};
    std::string customer;
    std::string name;
    std::string description;
    std::string startDate; // ISO format: YYYY-MM-DD
    std::string endDate;   // ISO format: YYYY-MM-DD
    core::ProjectStatus status{core::ProjectStatus::Planning};
    std::string scope;     // Markdown description of scope
    std::string targets;   // Markdown list of target IPs/domains
    std::string notes;     // Project-specific testing notes
};

} // namespace reportforge::models
