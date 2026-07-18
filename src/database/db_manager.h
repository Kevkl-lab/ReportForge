#pragma once

#include <QSqlDatabase>
#include <QString>

namespace reportforge::database {

/**
 * @brief Singleton class to manage SQLite database connection and migrations.
 */
class DbManager {
public:
    static DbManager& instance();
    ~DbManager();

    /**
     * @brief Open connection to the SQLite database and run migrations.
     * @param dbPath Path to the database file (defaults to reportforge.db).
     * @return true if successful, false otherwise.
     */
    bool initialize(const QString& dbPath = "reportforge.db");

    /**
     * @brief Retrieve the active QSqlDatabase instance.
     */
    QSqlDatabase database() const;

    /**
     * @brief Close the database connection.
     */
    void close();

private:
    DbManager() = default;
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    /**
     * @brief Execute schema migrations based on PRAGMA user_version.
     */
    bool runMigrations();

    QSqlDatabase db_;
};

} // namespace reportforge::database
