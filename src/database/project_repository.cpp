#include "project_repository.h"
#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace reportforge::database {

std::vector<models::Project> ProjectRepository::getAll() {
    std::vector<models::Project> projects;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query("SELECT id, customer, name, description, start_date, end_date, status, scope, targets, notes FROM projects ORDER BY id DESC;", db);

    while (query.next()) {
        models::Project project;
        project.id = query.value(0).toInt();
        project.customer = query.value(1).toString().toStdString();
        project.name = query.value(2).toString().toStdString();
        project.description = query.value(3).toString().toStdString();
        project.startDate = query.value(4).toString().toStdString();
        project.endDate = query.value(5).toString().toStdString();
        project.status = core::stringToProjectStatus(query.value(6).toString().toStdString());
        project.scope = query.value(7).toString().toStdString();
        project.targets = query.value(8).toString().toStdString();
        project.notes = query.value(9).toString().toStdString();
        projects.push_back(project);
    }
    return projects;
}

std::optional<models::Project> ProjectRepository::getById(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, customer, name, description, start_date, end_date, status, scope, targets, notes FROM projects WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        models::Project project;
        project.id = query.value(0).toInt();
        project.customer = query.value(1).toString().toStdString();
        project.name = query.value(2).toString().toStdString();
        project.description = query.value(3).toString().toStdString();
        project.startDate = query.value(4).toString().toStdString();
        project.endDate = query.value(5).toString().toStdString();
        project.status = core::stringToProjectStatus(query.value(6).toString().toStdString());
        project.scope = query.value(7).toString().toStdString();
        project.targets = query.value(8).toString().toStdString();
        project.notes = query.value(9).toString().toStdString();
        return project;
    }
    return std::nullopt;
}

bool ProjectRepository::insert(const models::Project& project) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO projects (customer, name, description, start_date, end_date, status, scope, targets, notes) "
                  "VALUES (:customer, :name, :description, :start_date, :end_date, :status, :scope, :targets, :notes);");
    query.bindValue(":customer", QString::fromStdString(project.customer));
    query.bindValue(":name", QString::fromStdString(project.name));
    query.bindValue(":description", QString::fromStdString(project.description));
    query.bindValue(":start_date", QString::fromStdString(project.startDate));
    query.bindValue(":end_date", QString::fromStdString(project.endDate));
    query.bindValue(":status", QString::fromStdString(core::projectStatusToString(project.status)));
    query.bindValue(":scope", QString::fromStdString(project.scope));
    query.bindValue(":targets", QString::fromStdString(project.targets));
    query.bindValue(":notes", QString::fromStdString(project.notes));

    if (!query.exec()) {
        qWarning() << "Failed to insert project:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectRepository::update(const models::Project& project) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("UPDATE projects SET customer = :customer, name = :name, description = :description, "
                  "start_date = :start_date, end_date = :end_date, status = :status, "
                  "scope = :scope, targets = :targets, notes = :notes WHERE id = :id;");
    query.bindValue(":id", project.id);
    query.bindValue(":customer", QString::fromStdString(project.customer));
    query.bindValue(":name", QString::fromStdString(project.name));
    query.bindValue(":description", QString::fromStdString(project.description));
    query.bindValue(":start_date", QString::fromStdString(project.startDate));
    query.bindValue(":end_date", QString::fromStdString(project.endDate));
    query.bindValue(":status", QString::fromStdString(core::projectStatusToString(project.status)));
    query.bindValue(":scope", QString::fromStdString(project.scope));
    query.bindValue(":targets", QString::fromStdString(project.targets));
    query.bindValue(":notes", QString::fromStdString(project.notes));

    if (!query.exec()) {
        qWarning() << "Failed to update project:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectRepository::remove(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM projects WHERE id = :id;");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete project:" << query.lastError().text();
        return false;
    }
    return true;
}

int ProjectRepository::getLastInsertedId() {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query("SELECT last_insert_rowid();", db);
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

} // namespace reportforge::database
