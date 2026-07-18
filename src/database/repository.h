#pragma once

#include <vector>
#include <optional>

namespace reportforge::database {

/**
 * @brief Generic Repository interface for CRUD operations.
 * @tparam T The model type.
 */
template <typename T>
class IRepository {
public:
    virtual ~IRepository() = default;

    /**
     * @brief Retrieve all records of type T.
     */
    virtual std::vector<T> getAll() = 0;

    /**
     * @brief Retrieve a record by its unique ID.
     */
    virtual std::optional<T> getById(int id) = 0;

    /**
     * @brief Insert a new record.
     * @return true if successful, false otherwise.
     */
    virtual bool insert(const T& entity) = 0;

    /**
     * @brief Update an existing record.
     * @return true if successful, false otherwise.
     */
    virtual bool update(const T& entity) = 0;

    /**
     * @brief Delete a record by ID.
     * @return true if successful, false otherwise.
     */
    virtual bool remove(int id) = 0;
};

} // namespace reportforge::database
