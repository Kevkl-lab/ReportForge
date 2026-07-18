#pragma once

#include <string>
#include <vector>

namespace reportforge::core {

enum class Severity {
    Critical,
    High,
    Medium,
    Low,
    Informational
};

enum class ProjectStatus {
    Planning,
    Testing,
    Retesting,
    Completed
};

enum class FindingStatus {
    Open,
    Fixed,
    Retested,
    AcceptedRisk
};

enum class MediaType {
    Image,
    PDF,
    Log,
    Text,
    Other
};

// Helper conversion functions
inline std::string severityToString(Severity severity) {
    switch (severity) {
        case Severity::Critical:      return "Critical";
        case Severity::High:          return "High";
        case Severity::Medium:        return "Medium";
        case Severity::Low:           return "Low";
        case Severity::Informational: return "Informational";
    }
    return "Unknown";
}

inline Severity stringToSeverity(const std::string& str) {
    if (str == "Critical")      return Severity::Critical;
    if (str == "High")          return Severity::High;
    if (str == "Medium")        return Severity::Medium;
    if (str == "Low")           return Severity::Low;
    return Severity::Informational;
}

inline std::string projectStatusToString(ProjectStatus status) {
    switch (status) {
        case ProjectStatus::Planning:  return "Planning";
        case ProjectStatus::Testing:   return "Testing";
        case ProjectStatus::Retesting: return "Retesting";
        case ProjectStatus::Completed: return "Completed";
    }
    return "Planning";
}

inline ProjectStatus stringToProjectStatus(const std::string& str) {
    if (str == "Testing")   return ProjectStatus::Testing;
    if (str == "Retesting") return ProjectStatus::Retesting;
    if (str == "Completed") return ProjectStatus::Completed;
    return ProjectStatus::Planning;
}

inline std::string findingStatusToString(FindingStatus status) {
    switch (status) {
        case FindingStatus::Open:         return "Open";
        case FindingStatus::Fixed:        return "Fixed";
        case FindingStatus::Retested:     return "Retested";
        case FindingStatus::AcceptedRisk: return "Accepted Risk";
    }
    return "Open";
}

inline FindingStatus stringToFindingStatus(const std::string& str) {
    if (str == "Fixed")         return FindingStatus::Fixed;
    if (str == "Retested")      return FindingStatus::Retested;
    if (str == "Accepted Risk") return FindingStatus::AcceptedRisk;
    if (str == "AcceptedRisk")  return FindingStatus::AcceptedRisk;
    return FindingStatus::Open;
}

inline std::string mediaTypeToString(MediaType type) {
    switch (type) {
        case MediaType::Image: return "Image";
        case MediaType::PDF:   return "PDF";
        case MediaType::Log:   return "Log";
        case MediaType::Text:  return "Text";
        case MediaType::Other: return "Other";
    }
    return "Other";
}

inline MediaType stringToMediaType(const std::string& str) {
    if (str == "Image") return MediaType::Image;
    if (str == "PDF")   return MediaType::PDF;
    if (str == "Log")   return MediaType::Log;
    if (str == "Text")  return MediaType::Text;
    return MediaType::Other;
}

} // namespace reportforge::core
