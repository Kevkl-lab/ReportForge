#include "import_export.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace reportforge::services {

static QJsonObject serializeProject(const models::Project& project) {
    QJsonObject obj;
    obj["customer"] = QString::fromStdString(project.customer);
    obj["name"] = QString::fromStdString(project.name);
    obj["description"] = QString::fromStdString(project.description);
    obj["start_date"] = QString::fromStdString(project.startDate);
    obj["end_date"] = QString::fromStdString(project.endDate);
    obj["status"] = QString::fromStdString(core::projectStatusToString(project.status));
    obj["scope"] = QString::fromStdString(project.scope);
    obj["targets"] = QString::fromStdString(project.targets);
    obj["notes"] = QString::fromStdString(project.notes);
    return obj;
}

static models::Project deserializeProject(const QJsonObject& obj) {
    models::Project project;
    project.customer = obj["customer"].toString().toStdString();
    project.name = obj["name"].toString().toStdString();
    project.description = obj["description"].toString().toStdString();
    project.startDate = obj["start_date"].toString().toStdString();
    project.endDate = obj["end_date"].toString().toStdString();
    project.status = core::stringToProjectStatus(obj["status"].toString().toStdString());
    project.scope = obj["scope"].toString().toStdString();
    project.targets = obj["targets"].toString().toStdString();
    project.notes = obj["notes"].toString().toStdString();
    return project;
}

static QJsonObject serializeFinding(const models::Finding& f) {
    QJsonObject obj;
    obj["title"] = QString::fromStdString(f.title);
    obj["severity"] = QString::fromStdString(core::severityToString(f.severity));
    obj["cvss_score"] = f.cvssScore;
    obj["owasp_category"] = QString::fromStdString(f.owaspCategory);
    obj["cwe"] = QString::fromStdString(f.cwe);
    obj["description"] = QString::fromStdString(f.description);
    obj["impact"] = QString::fromStdString(f.impact);
    obj["recommendation"] = QString::fromStdString(f.recommendation);
    obj["proof_of_concept"] = QString::fromStdString(f.proofOfConcept);
    obj["reproduction_steps"] = QString::fromStdString(f.reproductionSteps);
    obj["evidence"] = QString::fromStdString(f.evidence);
    obj["affected_assets"] = QString::fromStdString(f.affectedAssets);
    obj["status"] = QString::fromStdString(core::findingStatusToString(f.status));
    obj["tags"] = QString::fromStdString(f.tags);
    return obj;
}

static models::Finding deserializeFinding(const QJsonObject& obj) {
    models::Finding f;
    f.title = obj["title"].toString().toStdString();
    f.severity = core::stringToSeverity(obj["severity"].toString().toStdString());
    f.cvssScore = obj["cvss_score"].toDouble();
    f.owaspCategory = obj["owasp_category"].toString().toStdString();
    f.cwe = obj["cwe"].toString().toStdString();
    f.description = obj["description"].toString().toStdString();
    f.impact = obj["impact"].toString().toStdString();
    f.recommendation = obj["recommendation"].toString().toStdString();
    f.proofOfConcept = obj["proof_of_concept"].toString().toStdString();
    f.reproductionSteps = obj["reproduction_steps"].toString().toStdString();
    f.evidence = obj["evidence"].toString().toStdString();
    f.affectedAssets = obj["affected_assets"].toString().toStdString();
    f.status = core::stringToFindingStatus(obj["status"].toString().toStdString());
    f.tags = obj["tags"].toString().toStdString();
    return f;
}

bool ImportExportService::exportProjectJson(const std::string& filePath, const models::Project& project, const std::vector<models::Finding>& findings) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open export file:" << file.errorString();
        return false;
    }

    QJsonObject root;
    root["project"] = serializeProject(project);

    QJsonArray findingsArray;
    for (const auto& f : findings) {
        findingsArray.append(serializeFinding(f));
    }
    root["findings"] = findingsArray;

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ImportExportService::importProjectJson(const std::string& filePath, models::Project& outProject, std::vector<models::Finding>& outFindings) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open import file:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("project")) {
        qWarning() << "JSON lacks project data";
        return false;
    }

    outProject = deserializeProject(root["project"].toObject());
    outFindings.clear();

    if (root.contains("findings") && root["findings"].isArray()) {
        QJsonArray arr = root["findings"].toArray();
        for (int i = 0; i < arr.size(); ++i) {
            outFindings.push_back(deserializeFinding(arr[i].toObject()));
        }
    }

    return true;
}

bool ImportExportService::exportFindingTemplateJson(const std::string& filePath, const models::Finding& finding) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open export template file:" << file.errorString();
        return false;
    }

    QJsonObject root = serializeFinding(finding);
    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ImportExportService::importFindingTemplateJson(const std::string& filePath, models::Finding& outFinding) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open import template file:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (doc.isNull()) {
        qWarning() << "Failed to parse template JSON:" << error.errorString();
        return false;
    }

    outFinding = deserializeFinding(doc.object());
    return true;
}

} // namespace reportforge::services
