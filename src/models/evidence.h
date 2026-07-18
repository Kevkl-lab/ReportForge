#pragma once

#include <string>
#include "../core/enums.h"

namespace reportforge::models {

/**
 * @brief Represents evidence uploaded via drag & drop.
 */
struct Evidence {
    int id{0};
    int projectId{0};
    int findingId{0}; // 0 if project-level evidence, non-zero if linked to a finding
    std::string filePath;
    std::string fileName;
    core::MediaType fileType{core::MediaType::Other};
};

} // namespace reportforge::models
