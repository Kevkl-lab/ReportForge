#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

namespace reportforge::database {

DbManager& DbManager::instance() {
    static DbManager inst;
    return inst;
}

DbManager::~DbManager() {
    close();
}

bool DbManager::initialize(const QString& dbPath) {
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(dbPath);

    if (!db_.open()) {
        qCritical() << "Failed to open database:" << db_.lastError().text();
        return false;
    }

    // Enable foreign keys in SQLite
    QSqlQuery query("PRAGMA foreign_keys = ON;", db_);
    if (!query.exec()) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }

    return runMigrations();
}

QSqlDatabase DbManager::database() const {
    return db_;
}

void DbManager::close() {
    if (db_.isOpen()) {
        db_.close();
    }
}

bool DbManager::runMigrations() {
    QSqlQuery query(db_);

    // Get current version
    int version = 0;
    if (query.exec("PRAGMA user_version;") && query.next()) {
        version = query.value(0).toInt();
    }

    if (version < 1) {
        // Create projects table
        QString createProjects = 
            "CREATE TABLE IF NOT EXISTS projects ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  customer TEXT NOT NULL,"
            "  name TEXT NOT NULL,"
            "  description TEXT,"
            "  start_date TEXT,"
            "  end_date TEXT,"
            "  status TEXT NOT NULL,"
            "  scope TEXT,"
            "  targets TEXT,"
            "  notes TEXT"
            ");";
        if (!query.exec(createProjects)) {
            qCritical() << "Failed to create projects table:" << query.lastError().text();
            return false;
        }

        // Create findings table
        QString createFindings = 
            "CREATE TABLE IF NOT EXISTS findings ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  project_id INTEGER NOT NULL,"
            "  title TEXT NOT NULL,"
            "  severity TEXT NOT NULL,"
            "  cvss_score REAL,"
            "  owasp_category TEXT,"
            "  cwe TEXT,"
            "  description TEXT,"
            "  impact TEXT,"
            "  recommendation TEXT,"
            "  proof_of_concept TEXT,"
            "  reproduction_steps TEXT,"
            "  evidence TEXT,"
            "  affected_assets TEXT,"
            "  status TEXT NOT NULL,"
            "  tags TEXT,"
            "  FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE"
            ");";
        if (!query.exec(createFindings)) {
            qCritical() << "Failed to create findings table:" << query.lastError().text();
            return false;
        }

        // Create templates table
        QString createTemplates = 
            "CREATE TABLE IF NOT EXISTS templates ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  title TEXT NOT NULL,"
            "  severity TEXT NOT NULL,"
            "  cvss_score REAL,"
            "  owasp_category TEXT,"
            "  cwe TEXT,"
            "  description TEXT,"
            "  impact TEXT,"
            "  recommendation TEXT"
            ");";
        if (!query.exec(createTemplates)) {
            qCritical() << "Failed to create templates table:" << query.lastError().text();
            return false;
        }

        // Create timeline table
        QString createTimeline = 
            "CREATE TABLE IF NOT EXISTS timeline_events ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  project_id INTEGER NOT NULL,"
            "  timestamp TEXT NOT NULL,"
            "  event_text TEXT NOT NULL,"
            "  FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE"
            ");";
        if (!query.exec(createTimeline)) {
            qCritical() << "Failed to create timeline_events table:" << query.lastError().text();
            return false;
        }

        // Create evidence_media table
        QString createEvidence = 
            "CREATE TABLE IF NOT EXISTS evidence_media ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  project_id INTEGER NOT NULL,"
            "  finding_id INTEGER,"
            "  file_path TEXT NOT NULL,"
            "  file_name TEXT NOT NULL,"
            "  file_type TEXT NOT NULL,"
            "  FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE"
            ");";
        if (!query.exec(createEvidence)) {
            qCritical() << "Failed to create evidence_media table:" << query.lastError().text();
            return false;
        }

        // Seed default templates if table is empty
        QSqlQuery countQuery("SELECT COUNT(*) FROM templates;", db_);
        if (countQuery.next() && countQuery.value(0).toInt() == 0) {
            struct SeedTemplate {
                QString title;
                QString severity;
                double cvss;
                QString owasp;
                QString cwe;
                QString desc;
                QString impact;
                QString rec;
            };

            std::vector<SeedTemplate> seeds = {
                {
                    "SQL Injection (SQLi)", "Critical", 9.8, "A03:2021-Injection", "CWE-89",
                    "An input validation error allows an attacker to inject SQL commands into queries executed by the application.",
                    "Attackers can bypass authentication, read sensitive database contents, modify database data, and potentially execute administrative database actions (like reading files or executing commands on the underlying OS).",
                    "Use parameterized queries / prepared statements for all database access. Never concatenate user input directly into SQL strings. Implement input validation and limit database user privileges."
                },
                {
                    "Stored Cross-Site Scripting (XSS)", "High", 7.2, "A03:2021-Injection", "CWE-79",
                    "The application fails to sanitize user input before storing it in a persistent database. When this data is later retrieved and rendered on a page, the browser interprets it as executable javascript code.",
                    "Attackers can hijack user sessions, steal cookies, redirect victims to malicious sites, or perform actions on behalf of the user within their session context.",
                    "Implement context-aware output encoding (such as HTML encoding) before rendering dynamic data in the browser. Utilize a strong Content Security Policy (CSP) and set HttpOnly flags on session cookies."
                },
                {
                    "Insecure Direct Object References (IDOR)", "High", 8.1, "A01:2021-Broken Access Control", "CWE-639",
                    "The application references internal implementation objects (such as database keys or file paths) directly in URLs or API requests without verifying if the user has permission to access them.",
                    "Unauthenticated or unauthorized users can access, modify, or delete sensitive data of other users by altering the identifier parameter.",
                    "Implement robust object-level access control checks. Ensure that the server validates the requesting user's identity and privileges against the requested resource before serving or modifying it. Alternatively, use cryptographically secure random identifiers (GUIDs) instead of sequential integers."
                },
                {
                    "Cross-Site Request Forgery (CSRF)", "Medium", 6.5, "A01:2021-Broken Access Control", "CWE-352",
                    "The application fails to verify whether a state-changing HTTP request originated from a user intentionally clicking or executing it, or if it was triggered automatically by a third-party site while the user was logged in.",
                    "An attacker can trick a victim's browser into performing actions (such as changing email addresses or transferring funds) on the target site where the victim is currently authenticated.",
                    "Incorporate unique, cryptographically secure anti-CSRF tokens in all state-changing forms and API requests. Utilize the SameSite attribute (Lax or Strict) on cookies to restrict third-party cross-site transmission."
                },
                {
                    "Server-Side Request Forgery (SSRF)", "High", 8.6, "A10:2021-Server-Side Request Forgery", "CWE-918",
                    "The application accepts a URL or host parameter from the user and fetches the resource server-side without verifying or filtering the destination address.",
                    "Attackers can force the application server to scan internal networks, access restricted internal web panels, query cloud metadata services (e.g. AWS IMDSv2 at 169.254.169.254), or bypass firewall protections.",
                    "Implement strict whitelisting of allowed domain names and protocols. Block request parsing to loopback (127.0.0.1) and private IP ranges (RFC 1918). Run URL parsing and DNS resolution checks on the server to prevent DNS rebinding."
                },
                {
                    "Remote Code Execution (RCE)", "Critical", 9.8, "A03:2021-Injection", "CWE-94",
                    "The application evaluates untrusted input as code or passes it unsanitized to OS system execution shells (e.g., system, exec, eval).",
                    "Attackers can execute arbitrary command shells on the host operating system, leading to full server compromise, data extraction, and lateral movement within the network.",
                    "Avoid passing user input directly to system interpreters. If system execution is unavoidable, use strongly-typed API mappings, strict input validation against a rigid alphanumeric whitelist, and run the service under low-privilege isolated container accounts."
                },
                {
                    "Directory Traversal", "High", 7.5, "A01:2021-Broken Access Control", "CWE-22",
                    "The application uses user input to construct file paths for local file reading or writing operations without sanitizing directory traversal characters (e.g. '../').",
                    "Attackers can read arbitrary configuration files, environment variables, source code, or critical system files (like /etc/passwd) from the host filesystem.",
                    "Sanitize file paths by resolving paths to absolute canonical paths and verifying they reside within a designated directory. Preferably, refer to files using a safe key-value lookup system rather than direct path strings."
                },
                {
                    "Broken Authentication", "High", 8.8, "A07:2021-Identification and Authentication Failures", "CWE-287",
                    "The application's authentication mechanisms are incorrectly configured, permitting weak passwords, credential stuffing, session hijacking, or exposing passwords in URL parameters.",
                    "Attackers can compromise tester or user accounts, gain access to sensitive project reports, and manipulate assessments.",
                    "Implement multi-factor authentication (MFA). Enforce strong password complexity rules. Store password hashes using secure algorithms (Argon2id or bcrypt). Implement rate limiting on login endpoints."
                }
            };

            QSqlQuery seedQuery(db_);
            seedQuery.prepare("INSERT INTO templates (title, severity, cvss_score, owasp_category, cwe, description, impact, recommendation) "
                              "VALUES (:title, :severity, :cvss, :owasp, :cwe, :desc, :impact, :rec);");

            for (const auto& seed : seeds) {
                seedQuery.bindValue(":title", seed.title);
                seedQuery.bindValue(":severity", seed.severity);
                seedQuery.bindValue(":cvss", seed.cvss);
                seedQuery.bindValue(":owasp", seed.owasp);
                seedQuery.bindValue(":cwe", seed.cwe);
                seedQuery.bindValue(":desc", seed.desc);
                seedQuery.bindValue(":impact", seed.impact);
                seedQuery.bindValue(":rec", seed.rec);
                if (!seedQuery.exec()) {
                    qWarning() << "Failed to seed template:" << seed.title << seedQuery.lastError().text();
                }
            }
        }

        // Set version to 1
        query.exec("PRAGMA user_version = 1;");
    }

    return true;
}

} // namespace reportforge::database
