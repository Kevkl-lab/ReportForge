#include "file_utils.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QDebug>

namespace reportforge::utils {

bool FileUtils::ensureDirExists(const std::string& directoryPath) {
    QDir dir(QString::fromStdString(directoryPath));
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

std::string FileUtils::getEvidenceDirectory(int projectId) {
    // Storing evidence in a local subfolder in the application directory for easy tracking
    QDir appDir = QDir::current();
    QString evidencePath = appDir.filePath(QString("evidence/project_%1").arg(projectId));
    ensureDirExists(evidencePath.toStdString());
    return evidencePath.toStdString();
}

std::string FileUtils::copyEvidenceFile(int projectId, const std::string& sourcePath, const std::string& fileName) {
    QString destDir = QString::fromStdString(getEvidenceDirectory(projectId));
    
    QFile sourceFile(QString::fromStdString(sourcePath));
    if (!sourceFile.exists()) {
        qWarning() << "Source file does not exist:" << QString::fromStdString(sourcePath);
        return "";
    }

    // Create unique filename using short UUID to prevent overwriting files with the same name
    QFileInfo fileInfo(QString::fromStdString(fileName));
    QString suffix = fileInfo.suffix();
    QString baseName = fileInfo.completeBaseName();
    QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(6);
    QString newFileName = QString("%1_%2.%3").arg(baseName, uniqueId, suffix);

    QString destPath = QDir(destDir).filePath(newFileName);

    if (sourceFile.copy(destPath)) {
        return destPath.toStdString();
    } else {
        qWarning() << "Failed to copy file to:" << destPath << "Error:" << sourceFile.errorString();
        return "";
    }
}

} // namespace reportforge::utils
