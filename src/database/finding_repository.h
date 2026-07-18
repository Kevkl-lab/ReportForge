#pragma once

#include "repository.h"
#include "../models/finding.h"
#include "../models/evidence.h"

namespace reportforge::database {

/**
 * @brief Repository implementation for database operations on Findings and attached Evidence.
 */
class FindingRepository : public IRepository<models::Finding> {
public:
    FindingRepository() = default;
    ~FindingRepository() override = default;

    std::vector<models::Finding> getAll() override;
    std::optional<models::Finding> getById(int id) override;
    bool insert(const models::Finding& finding) override;
    bool update(const models::Finding& finding) override;
    bool remove(int id) override;

    // Helper methods for Dashboard and project-specific views
    std::vector<models::Finding> getByProjectId(int projectId);
    int getCountBySeverity(core::Severity severity);
    int getCountByStatus(core::FindingStatus status);

    // Evidence helper methods
    bool addEvidence(const models::Evidence& evidence);
    std::vector<models::Evidence> getEvidenceForFinding(int findingId);
    std::vector<models::Evidence> getEvidenceForProject(int projectId);
    bool removeEvidence(int id);
};

} // namespace reportforge::database
