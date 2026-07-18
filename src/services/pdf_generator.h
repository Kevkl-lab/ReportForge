#pragma once

#include <string>
#include <vector>
#include "../models/project.h"
#include "../models/finding.h"
#include "../models/evidence.h"

namespace reportforge::services {

/**
 * @brief Service responsible for rendering and exporting security assessment reports to PDF.
 */
class PdfGenerator {
public:
    /**
     * @brief Generates a styled PDF report.
     * @param outputPath Target file path for the PDF document.
     * @param project Target project metadata.
     * @param findings List of findings associated with the project.
     * @param evidenceList List of evidence files to embed inside findings.
     * @return true if PDF was written successfully, false otherwise.
     */
    static bool generateReport(const std::string& outputPath,
                               const models::Project& project,
                               const std::vector<models::Finding>& findings,
                               const std::vector<models::Evidence>& evidenceList);

    /**
     * @brief Validates findings and returns a list of warning messages for incomplete sections.
     */
    static std::vector<std::string> validateFindings(const std::vector<models::Finding>& findings,
                                                    const std::vector<models::Evidence>& evidenceList);
};

} // namespace reportforge::services
