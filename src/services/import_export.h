#pragma once

#include <string>
#include <vector>
#include "../models/project.h"
#include "../models/finding.h"

namespace reportforge::services {

/**
 * @brief Handles JSON serialization and deserialization of projects, findings, and templates.
 */
class ImportExportService {
public:
    /**
     * @brief Exports a project and all its findings to a single JSON file.
     */
    static bool exportProjectJson(const std::string& filePath, const models::Project& project, const std::vector<models::Finding>& findings);

    /**
     * @brief Imports a project and its findings from a JSON file.
     */
    static bool importProjectJson(const std::string& filePath, models::Project& outProject, std::vector<models::Finding>& outFindings);

    /**
     * @brief Exports a single finding as a reusable JSON template.
     */
    static bool exportFindingTemplateJson(const std::string& filePath, const models::Finding& finding);

    /**
     * @brief Imports a single finding from a JSON template file.
     */
    static bool importFindingTemplateJson(const std::string& filePath, models::Finding& outFinding);
};

} // namespace reportforge::services
