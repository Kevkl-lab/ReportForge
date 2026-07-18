#include "template_repository.h"
#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace reportforge::database {

static models::FindingTemplate mapTemplateFromQuery(const QSqlQuery& query) {
    models::FindingTemplate temp;
    temp.id = query.value(0).toInt();
    temp.title = query.value(1).toString().toStdString();
    temp.severity = core::stringToSeverity(query.value(2).toString().toStdString());
    temp.cvssScore = query.value(3).toDouble();
    temp.owaspCategory = query.value(4).toString().toStdString();
    temp.cwe = query.value(5).toString().toStdString();
    temp.description = query.value(6).toString().toStdString();
    temp.impact = query.value(7).toString().toStdString();
    temp.recommendation = query.value(8).toString().toStdString();
    return temp;
}

std::vector<models::FindingTemplate> TemplateRepository::getAll() {
    std::vector<models::FindingTemplate> templates;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query("SELECT id, title, severity, cvss_score, owasp_category, cwe, description, impact, recommendation FROM templates ORDER BY title ASC;", db);

    while (query.next()) {
        templates.push_back(mapTemplateFromQuery(query));
    }
    return templates;
}

std::optional<models::FindingTemplate> TemplateRepository::getById(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, title, severity, cvss_score, owasp_category, cwe, description, impact, recommendation FROM templates WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return mapTemplateFromQuery(query);
    }
    return std::nullopt;
}

bool TemplateRepository::insert(const models::FindingTemplate& temp) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO templates (title, severity, cvss_score, owasp_category, cwe, description, impact, recommendation) "
                  "VALUES (:title, :severity, :cvss_score, :owasp_category, :cwe, :description, :impact, :recommendation);");
    query.bindValue(":title", QString::fromStdString(temp.title));
    query.bindValue(":severity", QString::fromStdString(core::severityToString(temp.severity)));
    query.bindValue(":cvss_score", temp.cvssScore);
    query.bindValue(":owasp_category", QString::fromStdString(temp.owaspCategory));
    query.bindValue(":cwe", QString::fromStdString(temp.cwe));
    query.bindValue(":description", QString::fromStdString(temp.description));
    query.bindValue(":impact", QString::fromStdString(temp.impact));
    query.bindValue(":recommendation", QString::fromStdString(temp.recommendation));

    if (!query.exec()) {
        qWarning() << "Failed to insert template:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TemplateRepository::update(const models::FindingTemplate& temp) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("UPDATE templates SET title = :title, severity = :severity, cvss_score = :cvss_score, "
                  "owasp_category = :owasp_category, cwe = :cwe, description = :description, "
                  "impact = :impact, recommendation = :recommendation WHERE id = :id;");
    query.bindValue(":id", temp.id);
    query.bindValue(":title", QString::fromStdString(temp.title));
    query.bindValue(":severity", QString::fromStdString(core::severityToString(temp.severity)));
    query.bindValue(":cvss_score", temp.cvssScore);
    query.bindValue(":owasp_category", QString::fromStdString(temp.owaspCategory));
    query.bindValue(":cwe", QString::fromStdString(temp.cwe));
    query.bindValue(":description", QString::fromStdString(temp.description));
    query.bindValue(":impact", QString::fromStdString(temp.impact));
    query.bindValue(":recommendation", QString::fromStdString(temp.recommendation));

    if (!query.exec()) {
        qWarning() << "Failed to update template:" << query.lastError().text();
        return false;
    }
    return true;
}

bool TemplateRepository::remove(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM templates WHERE id = :id;");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete template:" << query.lastError().text();
        return false;
    }
    return true;
}

} // namespace reportforge::database
