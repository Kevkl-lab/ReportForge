#pragma once

#include "repository.h"
#include "../models/timeline.h"

namespace reportforge::database {

/**
 * @brief Repository implementation for database operations on TimelineEvents.
 */
class TimelineRepository : public IRepository<models::TimelineEvent> {
public:
    TimelineRepository() = default;
    ~TimelineRepository() override = default;

    std::vector<models::TimelineEvent> getAll() override;
    std::optional<models::TimelineEvent> getById(int id) override;
    bool insert(const models::TimelineEvent& event) override;
    bool update(const models::TimelineEvent& event) override;
    bool remove(int id) override;

    // Helper methods for project timeline
    std::vector<models::TimelineEvent> getByProjectId(int projectId);
    bool logEvent(int projectId, const std::string& eventText);
};

} // namespace reportforge::database
