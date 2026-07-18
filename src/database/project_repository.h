#pragma once

#include "repository.h"
#include "../models/project.h"

namespace reportforge::database {

/**
 * @brief Repository implementation for database operations on Projects.
 */
class ProjectRepository : public IRepository<models::Project> {
public:
    ProjectRepository() = default;
    ~ProjectRepository() override = default;

    std::vector<models::Project> getAll() override;
    std::optional<models::Project> getById(int id) override;
    bool insert(const models::Project& project) override;
    bool update(const models::Project& project) override;
    bool remove(int id) override;
    
    // Custom query to get last inserted project ID
    int getLastInsertedId();
};

} // namespace reportforge::database
