#pragma once

#include "repository.h"
#include "../models/template.h"

namespace reportforge::database {

/**
 * @brief Repository implementation for database operations on FindingTemplates.
 */
class TemplateRepository : public IRepository<models::FindingTemplate> {
public:
    TemplateRepository() = default;
    ~TemplateRepository() override = default;

    std::vector<models::FindingTemplate> getAll() override;
    std::optional<models::FindingTemplate> getById(int id) override;
    bool insert(const models::FindingTemplate& temp) override;
    bool update(const models::FindingTemplate& temp) override;
    bool remove(int id) override;
};

} // namespace reportforge::database
