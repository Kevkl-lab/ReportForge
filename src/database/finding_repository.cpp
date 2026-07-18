#include "finding_repository.h"
#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

namespace reportforge::database {

static models::Finding mapFindingFromQuery(const QSqlQuery& query) {
    models::Finding finding;
    finding.id = query.value(0).toInt();
    finding.projectId = query.value(1).toInt();
    finding.title = query.value(2).toString().toStdString();
    finding.severity = core::stringToSeverity(query.value(3).toString().toStdString());
    finding.cvssScore = query.value(4).toDouble();
    finding.owaspCategory = query.value(5).toString().toStdString();
    finding.cwe = query.value(6).toString().toStdString();
    finding.description = query.value(7).toString().toStdString();
    finding.impact = query.value(8).toString().toStdString();
    finding.recommendation = query.value(9).toString().toStdString();
    finding.proofOfConcept = query.value(10).toString().toStdString();
    finding.reproductionSteps = query.value(11).toString().toStdString();
    finding.evidence = query.value(12).toString().toStdString();
    finding.affectedAssets = query.value(13).toString().toStdString();
    finding.status = core::stringToFindingStatus(query.value(14).toString().toStdString());
    finding.tags = query.value(15).toString().toStdString();
    return finding;
}

std::vector<models::Finding> FindingRepository::getAll() {
    std::vector<models::Finding> findings;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query("SELECT id, project_id, title, severity, cvss_score, owasp_category, cwe, description, "
                    "impact, recommendation, proof_of_concept, reproduction_steps, evidence, affected_assets, "
                    "status, tags FROM findings ORDER BY id DESC;", db);

    while (query.next()) {
        findings.push_back(mapFindingFromQuery(query));
    }
    return findings;
}

std::optional<models::Finding> FindingRepository::getById(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, title, severity, cvss_score, owasp_category, cwe, description, "
                  "impact, recommendation, proof_of_concept, reproduction_steps, evidence, affected_assets, "
                  "status, tags FROM findings WHERE id = :id;");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return mapFindingFromQuery(query);
    }
    return std::nullopt;
}

bool FindingRepository::insert(const models::Finding& finding) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO findings (project_id, title, severity, cvss_score, owasp_category, cwe, "
                  "description, impact, recommendation, proof_of_concept, reproduction_steps, evidence, "
                  "affected_assets, status, tags) "
                  "VALUES (:project_id, :title, :severity, :cvss_score, :owasp_category, :cwe, "
                  ":description, :impact, :recommendation, :proof_of_concept, :reproduction_steps, :evidence, "
                  ":affected_assets, :status, :tags);");
    query.bindValue(":project_id", finding.projectId);
    query.bindValue(":title", QString::fromStdString(finding.title));
    query.bindValue(":severity", QString::fromStdString(core::severityToString(finding.severity)));
    query.bindValue(":cvss_score", finding.cvssScore);
    query.bindValue(":owasp_category", QString::fromStdString(finding.owaspCategory));
    query.bindValue(":cwe", QString::fromStdString(finding.cwe));
    query.bindValue(":description", QString::fromStdString(finding.description));
    query.bindValue(":impact", QString::fromStdString(finding.impact));
    query.bindValue(":recommendation", QString::fromStdString(finding.recommendation));
    query.bindValue(":proof_of_concept", QString::fromStdString(finding.proofOfConcept));
    query.bindValue(":reproduction_steps", QString::fromStdString(finding.reproductionSteps));
    query.bindValue(":evidence", QString::fromStdString(finding.evidence));
    query.bindValue(":affected_assets", QString::fromStdString(finding.affectedAssets));
    query.bindValue(":status", QString::fromStdString(core::findingStatusToString(finding.status)));
    query.bindValue(":tags", QString::fromStdString(finding.tags));

    if (!query.exec()) {
        qWarning() << "Failed to insert finding:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FindingRepository::update(const models::Finding& finding) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("UPDATE findings SET project_id = :project_id, title = :title, severity = :severity, "
                  "cvss_score = :cvss_score, owasp_category = :owasp_category, cwe = :cwe, "
                  "description = :description, impact = :impact, recommendation = :recommendation, "
                  "proof_of_concept = :proof_of_concept, reproduction_steps = :reproduction_steps, "
                  "evidence = :evidence, affected_assets = :affected_assets, status = :status, tags = :tags "
                  "WHERE id = :id;");
    query.bindValue(":id", finding.id);
    query.bindValue(":project_id", finding.projectId);
    query.bindValue(":title", QString::fromStdString(finding.title));
    query.bindValue(":severity", QString::fromStdString(core::severityToString(finding.severity)));
    query.bindValue(":cvss_score", finding.cvssScore);
    query.bindValue(":owasp_category", QString::fromStdString(finding.owaspCategory));
    query.bindValue(":cwe", QString::fromStdString(finding.cwe));
    query.bindValue(":description", QString::fromStdString(finding.description));
    query.bindValue(":impact", QString::fromStdString(finding.impact));
    query.bindValue(":recommendation", QString::fromStdString(finding.recommendation));
    query.bindValue(":proof_of_concept", QString::fromStdString(finding.proofOfConcept));
    query.bindValue(":reproduction_steps", QString::fromStdString(finding.reproductionSteps));
    query.bindValue(":evidence", QString::fromStdString(finding.evidence));
    query.bindValue(":affected_assets", QString::fromStdString(finding.affectedAssets));
    query.bindValue(":status", QString::fromStdString(core::findingStatusToString(finding.status)));
    query.bindValue(":tags", QString::fromStdString(finding.tags));

    if (!query.exec()) {
        qWarning() << "Failed to update finding:" << query.lastError().text();
        return false;
    }
    return true;
}

bool FindingRepository::remove(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM findings WHERE id = :id;");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete finding:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<models::Finding> FindingRepository::getByProjectId(int projectId) {
    std::vector<models::Finding> findings;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, title, severity, cvss_score, owasp_category, cwe, description, "
                  "impact, recommendation, proof_of_concept, reproduction_steps, evidence, affected_assets, "
                  "status, tags FROM findings WHERE project_id = :project_id ORDER BY severity ASC, cvss_score DESC;");
    query.bindValue(":project_id", projectId);

    if (query.exec()) {
        while (query.next()) {
            findings.push_back(mapFindingFromQuery(query));
        }
    } else {
        qWarning() << "Failed to get findings by project ID:" << query.lastError().text();
    }
    return findings;
}

int FindingRepository::getCountBySeverity(core::Severity severity) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM findings WHERE severity = :severity;");
    query.bindValue(":severity", QString::fromStdString(core::severityToString(severity)));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int FindingRepository::getCountByStatus(core::FindingStatus status) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM findings WHERE status = :status;");
    query.bindValue(":status", QString::fromStdString(core::findingStatusToString(status)));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool FindingRepository::addEvidence(const models::Evidence& ev) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO evidence_media (project_id, finding_id, file_path, file_name, file_type) "
                  "VALUES (:project_id, :finding_id, :file_path, :file_name, :file_type);");
    query.bindValue(":project_id", ev.projectId);
    query.bindValue(":finding_id", ev.findingId > 0 ? QVariant(ev.findingId) : QVariant(QVariant::Int));
    query.bindValue(":file_path", QString::fromStdString(ev.filePath));
    query.bindValue(":file_name", QString::fromStdString(ev.fileName));
    query.bindValue(":file_type", QString::fromStdString(core::mediaTypeToString(ev.fileType)));
    
    if (!query.exec()) {
        qWarning() << "Failed to insert evidence:" << query.lastError().text();
        return false;
    }
    return true;
}

std::vector<models::Evidence> FindingRepository::getEvidenceForFinding(int findingId) {
    std::vector<models::Evidence> results;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, finding_id, file_path, file_name, file_type FROM evidence_media WHERE finding_id = :finding_id;");
    query.bindValue(":finding_id", findingId);
    
    if (query.exec()) {
        while (query.next()) {
            models::Evidence ev;
            ev.id = query.value(0).toInt();
            ev.projectId = query.value(1).toInt();
            ev.findingId = query.value(2).toInt();
            ev.filePath = query.value(3).toString().toStdString();
            ev.fileName = query.value(4).toString().toStdString();
            ev.fileType = core::stringToMediaType(query.value(5).toString().toStdString());
            results.push_back(ev);
        }
    }
    return results;
}

std::vector<models::Evidence> FindingRepository::getEvidenceForProject(int projectId) {
    std::vector<models::Evidence> results;
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, project_id, finding_id, file_path, file_name, file_type FROM evidence_media WHERE project_id = :project_id;");
    query.bindValue(":project_id", projectId);
    
    if (query.exec()) {
        while (query.next()) {
            models::Evidence ev;
            ev.id = query.value(0).toInt();
            ev.projectId = query.value(1).toInt();
            ev.findingId = query.value(2).toInt();
            ev.filePath = query.value(3).toString().toStdString();
            ev.fileName = query.value(4).toString().toStdString();
            ev.fileType = core::stringToMediaType(query.value(5).toString().toStdString());
            results.push_back(ev);
        }
    }
    return results;
}

bool FindingRepository::removeEvidence(int id) {
    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM evidence_media WHERE id = :id;");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete evidence:" << query.lastError().text();
        return false;
    }
    return true;
}

} // namespace reportforge::database
