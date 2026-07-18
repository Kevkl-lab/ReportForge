#pragma once

#include <string>

namespace reportforge::models {

/**
 * @brief Represents an event in a project's activity timeline.
 */
struct TimelineEvent {
    int id{0};
    int projectId{0};
    std::string timestamp; // ISO format or time string e.g. "09:15"
    std::string eventText;
};

} // namespace reportforge::models
