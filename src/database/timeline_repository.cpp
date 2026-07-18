#include "timeline_repository.h"
#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

namespace reportforge::database {

std::vector<models::TimelineEvent> TimelineRepository::getAll() {
    std::vector<models::TimelineEvent> events;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query("SELECT id, project_id, timestamp, event_text FROM timeline_events ORDER BY id ASC;", db);

    while (query.next()) {
        models::TimelineEvent event;
        event.id = query.value(0).toInt();
        event.projectId = query.value(1).toInt();
        event.timestamp = query.value(2).toString().toStdString();
        event.eventText = query.value(3).toString().toStdString();
        events.push_back(event);
    }
    return events;
}

std::optional<models::TimelineEvent> TimelineRepository::getById(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, timestamp, event_text FROM timeline_events WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        models::TimelineEvent event;
        event.id = query.value(0).toInt();
        event.projectId = query.value(1).toInt();
        event.timestamp = query.value(2).toString().toStdString();
        event.eventText = query.value(3).toString().toStdString();
        return event;
    }
    return std::nullopt;
}

bool TimelineRepository::insert(const models::TimelineEvent& event) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO timeline_events (project_id, timestamp, event_text) "
                  "VALUES (:project_id, :timestamp, :event_text);");
    query.bindValue(":project_id", event.projectId);
    query.bindValue(":timestamp", QString::fromStdString(event.timestamp));
    query.bindValue(":event_text", QString::fromStdString(event.eventText));

    if (!query.exec()) {
        qWarning() << "Failed to insert timeline event:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TimelineRepository::update(const models::TimelineEvent& event) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("UPDATE timeline_events SET project_id = :project_id, timestamp = :timestamp, event_text = :event_text WHERE id = :id;");
    query.bindValue(":id", event.id);
    query.bindValue(":project_id", event.projectId);
    query.bindValue(":timestamp", QString::fromStdString(event.timestamp));
    query.bindValue(":event_text", QString::fromStdString(event.eventText));

    if (!query.exec()) {
        qWarning() << "Failed to update timeline event:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TimelineRepository::remove(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM timeline_events WHERE id = :id;");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete timeline event:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<models::TimelineEvent> TimelineRepository::getByProjectId(int projectId) {
    std::vector<models::TimelineEvent> events;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, timestamp, event_text FROM timeline_events WHERE project_id = :project_id ORDER BY id ASC;");
    query.bindValue(":project_id", projectId);

    if (query.exec()) {
        while (query.next()) {
            models::TimelineEvent event;
            event.id = query.value(0).toInt();
            event.projectId = query.value(1).toInt();
            event.timestamp = query.value(2).toString().toStdString();
            event.eventText = query.value(3).toString().toStdString();
            events.push_back(event);
        }
    }
    return events;
}

bool TimelineRepository::logEvent(int projectId, const std::string& eventText) {
    models::TimelineEvent event;
    event.projectId = projectId;
    event.timestamp = QDateTime::currentDateTime().toString("hh:mm").toStdString();
    event.eventText = eventText;
    return insert(event);
}

} // namespace reportforge::database
