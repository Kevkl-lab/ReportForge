#pragma once

#include <string>

namespace reportforge::utils {

/**
 * @brief Utilities for managing files and directory structures for evidence attachments.
 */
class FileUtils {
public:
    /**
     * @brief Ensures a directory exists, creating nested directories if required.
     */
    static bool ensureDirExists(const std::string& directoryPath);

    /**
     * @brief Retrieves the project-specific evidence storage directory.
     */
    static std::string getEvidenceDirectory(int projectId);

    /**
     * @brief Copies an uploaded file into the project's evidence directory.
     * @return Path to the newly created copy, or empty string on failure.
     */
    static std::string copyEvidenceFile(int projectId, const std::string& sourcePath, const std::string& fileName);
};

} // namespace reportforge::utils
