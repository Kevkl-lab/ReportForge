#include "project_detail_view.h"
#include "../database/project_repository.h"
#include "../database/finding_repository.h"
#include "../database/timeline_repository.h"
#include "../services/pdf_generator.h"
#include "../services/import_export.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

namespace reportforge::widgets {

ProjectDetailView::ProjectDetailView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Back bar layout
    auto* topBar = new QHBoxLayout();
    
    auto* backBtn = new QPushButton("< Back to Projects", this);
    backBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #94a3b8;"
        "  border: 1px solid #2d2d34;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  color: #a78bfa;"
        "  border: 1px solid #8b5cf6;"
        "}"
    );
    topBar->addWidget(backBtn);
    topBar->addStretch();

    auto* editProjBtn = new QPushButton("Edit Project Info", this);
    editProjBtn->setStyleSheet(
        "QPushButton {"
        "  background: #232329;"
        "  color: #e2e8f0;"
        "  border: 1px solid #2d2d34;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "  color: #a78bfa;"
        "  border: 1px solid #8b5cf6;"
        "}"
    );
    topBar->addWidget(editProjBtn);

    auto* deleteProjBtn = new QPushButton("Delete Project", this);
    deleteProjBtn->setStyleSheet(
        "QPushButton {"
        "  background: #450a0a;"
        "  color: #fca5a5;"
        "  border: 1px solid #991b1b;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "  background: #7f1d1d;"
        "}"
    );
    topBar->addWidget(deleteProjBtn);
    mainLayout->addLayout(topBar);

    // Summary Card (Metadata Header)
    auto* summaryCard = new QFrame(this);
    summaryCard->setStyleSheet(
        "QFrame {"
        "  background-color: #16161e;"
        "  border: 1px solid #23232f;"
        "  border-radius: 8px;"
        "  padding: 15px;"
        "}"
    );
    auto* summaryLayout = new QHBoxLayout(summaryCard);
    
    auto* nameLayout = new QVBoxLayout();
    projNameLabel_ = new QLabel("Project Title", this);
    projNameLabel_->setStyleSheet("color: #e2e8f0; font-size: 18px; font-weight: bold; border: none;");
    custLabel_ = new QLabel("Customer Name", this);
    custLabel_->setStyleSheet("color: #94a3b8; font-size: 13px; border: none;");
    nameLayout->addWidget(projNameLabel_);
    nameLayout->addWidget(custLabel_);
    summaryLayout->addLayout(nameLayout, 2);

    auto* metaLayout = new QVBoxLayout();
    datesLabel_ = new QLabel("Dates: -- to --", this);
    datesLabel_->setStyleSheet("color: #64748b; font-size: 11px; border: none;");
    statusLabel_ = new QLabel("Status: PLANNING", this);
    statusLabel_->setStyleSheet("color: #38bdf8; font-size: 12px; font-weight: bold; border: none;");
    metaLayout->addWidget(datesLabel_);
    metaLayout->addWidget(statusLabel_);
    summaryLayout->addLayout(metaLayout, 1);
    
    mainLayout->addWidget(summaryCard);

    // Tabs setup
    tabWidget_ = new QTabWidget(this);
    tabWidget_->setStyleSheet(
        "QTabWidget::pane {"
        "  border: 1px solid #23232f;"
        "  background: #111115;"
        "  border-radius: 8px;"
        "}"
        "QTabBar::tab {"
        "  background: #16161e;"
        "  color: #94a3b8;"
        "  padding: 8px 16px;"
        "  margin-right: 4px;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "  border: 1px solid #23232f;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #111115;"
        "  color: #a78bfa;"
        "  border-bottom: 2px solid #8b5cf6;"
        "}"
    );

    // ----------------------------------------------------
    // TAB 1: SCOPE & TARGETS
    // ----------------------------------------------------
    auto* scopeTab = new QWidget(this);
    auto* scopeLayout = new QVBoxLayout(scopeTab);
    scopeLayout->setContentsMargins(20, 20, 20, 20);
    scopeLayout->setSpacing(15);

    auto* editorsLayout = new QHBoxLayout();
    
    auto* scopeSub = new QVBoxLayout();
    auto* scopeTitle = new QLabel("Assessment Scope (Markdown)", this);
    scopeTitle->setStyleSheet("color: #a78bfa; font-weight: bold; font-size: 11px;");
    scopeEditor_ = new MarkdownEditor(this);
    scopeEditor_->setPlaceholderText("Enter testing constraints, black box / white box details...");
    scopeSub->addWidget(scopeTitle);
    scopeSub->addWidget(scopeEditor_);
    editorsLayout->addLayout(scopeSub, 1);

    auto* targetsSub = new QVBoxLayout();
    auto* targetsTitle = new QLabel("Target Scope / IPs (Markdown)", this);
    targetsTitle->setStyleSheet("color: #a78bfa; font-weight: bold; font-size: 11px;");
    targetsEditor_ = new MarkdownEditor(this);
    targetsEditor_->setPlaceholderText("List target IP subnets, domains, API endpoints...");
    targetsSub->addWidget(targetsTitle);
    targetsSub->addWidget(targetsEditor_);
    editorsLayout->addLayout(targetsSub, 1);

    scopeLayout->addLayout(editorsLayout);

    auto* saveScopeBtn = new QPushButton("Save Scope && Targets", this);
    saveScopeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    scopeLayout->addWidget(saveScopeBtn);
    tabWidget_->addTab(scopeTab, "Scope");

    // ----------------------------------------------------
    // TAB 2: FINDINGS LOG
    // ----------------------------------------------------
    auto* findingsTab = new QWidget(this);
    auto* findingsLayout = new QVBoxLayout(findingsTab);
    findingsLayout->setContentsMargins(20, 20, 20, 20);
    findingsLayout->setSpacing(15);

    auto* fHeader = new QHBoxLayout();
    auto* fTitle = new QLabel("FINDINGS LOGGED", this);
    fTitle->setStyleSheet("color: #e2e8f0; font-weight: bold;");
    addFindingBtn_ = new QPushButton("+ Add Finding", this);
    addFindingBtn_->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    fHeader->addWidget(fTitle);
    fHeader->addStretch();
    fHeader->addWidget(addFindingBtn_);
    findingsLayout->addLayout(fHeader);

    findingsTable_ = new QTableWidget(this);
    findingsTable_->setColumnCount(6);
    findingsTable_->setHorizontalHeaderLabels({"ID", "Finding Title", "Severity", "CVSS Score", "Status", "CWE"});
    findingsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    findingsTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    findingsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    findingsTable_->verticalHeader()->setVisible(false);
    findingsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    findingsTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    findingsTable_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "  background-color: #111115;"
        "  color: #a78bfa;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #23232f;"
        "}"
    );
    findingsTable_->setStyleSheet(
        "QTableWidget {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  gridline-color: #1f1f2a;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "}"
        "QTableWidget::item { padding: 8px; }"
        "QTableWidget::item:selected { background-color: #232332; color: #a78bfa; }"
    );
    findingsLayout->addWidget(findingsTable_);
    tabWidget_->addTab(findingsTab, "Findings");

    // ----------------------------------------------------
    // TAB 3: TIMELINE
    // ----------------------------------------------------
    auto* timelineTab = new QWidget(this);
    auto* timelineLayout = new QVBoxLayout(timelineTab);
    timelineLayout->setContentsMargins(20, 20, 20, 20);
    timelineLayout->setSpacing(15);

    timelineList_ = new QListWidget(this);
    timelineList_->setStyleSheet(
        "QListWidget {"
        "  background-color: #16161e;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "  color: #e2e8f0;"
        "  padding: 10px;"
        "}"
        "QListWidget::item {"
        "  padding: 10px;"
        "  border-bottom: 1px solid #1f1f2a;"
        "}"
    );
    timelineLayout->addWidget(timelineList_);

    auto* timelineInputLayout = new QHBoxLayout();
    newEventInput_ = new QLineEdit(this);
    newEventInput_->setPlaceholderText("e.g. Recon started, Nmap Scan completed...");
    newEventInput_->setStyleSheet(
        "QLineEdit {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "  padding: 8px;"
        "}"
    );
    addEventBtn_ = new QPushButton("Log Event", this);
    addEventBtn_->setStyleSheet(
        "QPushButton {"
        "  background-color: #232329;"
        "  color: #e2e8f0;"
        "  border: 1px solid #2d2d34;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "  color: #a78bfa;"
        "  border-color: #8b5cf6;"
        "}"
    );
    timelineInputLayout->addWidget(newEventInput_, 1);
    timelineInputLayout->addWidget(addEventBtn_);
    timelineLayout->addLayout(timelineInputLayout);
    tabWidget_->addTab(timelineTab, "Timeline");

    // ----------------------------------------------------
    // TAB 4: PROJECT NOTES
    // ----------------------------------------------------
    auto* notesTab = new QWidget(this);
    auto* notesLayout = new QVBoxLayout(notesTab);
    notesLayout->setContentsMargins(20, 20, 20, 20);
    notesLayout->setSpacing(15);

    notesEditor_ = new MarkdownEditor(this);
    notesEditor_->setPlaceholderText("Keep internal project findings, scratchpad details here...");
    notesLayout->addWidget(notesEditor_);

    auto* saveNotesBtn = new QPushButton("Save Notes", this);
    saveNotesBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    notesLayout->addWidget(saveNotesBtn);
    tabWidget_->addTab(notesTab, "Project Notes");

    // ----------------------------------------------------
    // TAB 5: PDF & JSON GENERATE
    // ----------------------------------------------------
    auto* reportTab = new QWidget(this);
    auto* reportLayout = new QVBoxLayout(reportTab);
    reportLayout->setContentsMargins(20, 20, 20, 20);
    reportLayout->setSpacing(25);

    // PDF Group
    auto* pdfGroup = new QGroupBox("Generate PDF Report", this);
    pdfGroup->setStyleSheet("QGroupBox { color: #a78bfa; font-weight: bold; border: 1px solid #23232f; border-radius: 6px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; }");
    auto* pdfForm = new QFormLayout(pdfGroup);
    pdfPathInput_ = new QLineEdit(pdfGroup);
    pdfPathInput_->setPlaceholderText("/absolute/path/to/report.pdf");
    pdfPathInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #23232f; padding: 8px; border-radius: 4px; }");
    
    auto* browsePdfBtn = new QPushButton("Browse...", pdfGroup);
    browsePdfBtn->setStyleSheet("QPushButton { background: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px 12px; border-radius: 4px; } QPushButton:hover { color: #a78bfa; }");
    
    auto* pdfHBox = new QHBoxLayout();
    pdfHBox->addWidget(pdfPathInput_, 1);
    pdfHBox->addWidget(browsePdfBtn);
    pdfForm->addRow("Output PDF Path:", pdfHBox);

    auto* runPdfBtn = new QPushButton("Generate Beautiful PDF", pdfGroup);
    runPdfBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    pdfForm->addRow("", runPdfBtn);
    reportLayout->addWidget(pdfGroup);

    // JSON Group
    auto* jsonGroup = new QGroupBox("Data Import / Export (JSON)", this);
    jsonGroup->setStyleSheet("QGroupBox { color: #a78bfa; font-weight: bold; border: 1px solid #23232f; border-radius: 6px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; }");
    auto* jsonForm = new QFormLayout(jsonGroup);
    
    jsonPathInput_ = new QLineEdit(jsonGroup);
    jsonPathInput_->setPlaceholderText("/absolute/path/to/backup.json");
    jsonPathInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #23232f; padding: 8px; border-radius: 4px; }");
    
    auto* browseJsonBtn = new QPushButton("Browse...", jsonGroup);
    browseJsonBtn->setStyleSheet("QPushButton { background: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px 12px; border-radius: 4px; } QPushButton:hover { color: #a78bfa; }");
    
    auto* jsonHBox = new QHBoxLayout();
    jsonHBox->addWidget(jsonPathInput_, 1);
    jsonHBox->addWidget(browseJsonBtn);
    jsonForm->addRow("JSON Data Path:", jsonHBox);

    auto* actionHBox = new QHBoxLayout();
    auto* exportBtn = new QPushButton("Export Project JSON", jsonGroup);
    exportBtn->setStyleSheet("QPushButton { background: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 8px 16px; border-radius: 6px; font-weight: bold; } QPushButton:hover { color: #a78bfa; border-color: #8b5cf6; }");
    auto* importBtn = new QPushButton("Import & Merge JSON", jsonGroup);
    importBtn->setStyleSheet("QPushButton { background: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 8px 16px; border-radius: 6px; font-weight: bold; } QPushButton:hover { color: #a78bfa; border-color: #8b5cf6; }");
    actionHBox->addWidget(exportBtn);
    actionHBox->addWidget(importBtn);
    jsonForm->addRow("", actionHBox);

    reportLayout->addWidget(jsonGroup);
    reportLayout->addStretch();
    tabWidget_->addTab(reportTab, "Generate Report");

    mainLayout->addWidget(tabWidget_, 1);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &ProjectDetailView::backRequested);
    connect(editProjBtn, &QPushButton::clicked, this, [this]() { emit editProjectRequested(projectId_); });
    connect(deleteProjBtn, &QPushButton::clicked, this, &ProjectDetailView::onDeleteProject);
    
    connect(saveScopeBtn, &QPushButton::clicked, this, &ProjectDetailView::onSaveScopeAndTargets);
    connect(saveNotesBtn, &QPushButton::clicked, this, &ProjectDetailView::onSaveNotes);

    connect(addFindingBtn_, &QPushButton::clicked, this, [this]() { emit createFindingRequested(projectId_); });
    connect(findingsTable_, &QTableWidget::cellDoubleClicked, this, &ProjectDetailView::onFindingDoubleClicked);

    connect(addEventBtn_, &QPushButton::clicked, this, &ProjectDetailView::onAddTimelineEvent);
    connect(newEventInput_, &QLineEdit::returnPressed, this, &ProjectDetailView::onAddTimelineEvent);

    // File Dialog browsing
    connect(browsePdfBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Select PDF Output Path", QDir::homePath(), "PDF Files (*.pdf)");
        if (!path.isEmpty()) pdfPathInput_->setText(path);
    });
    connect(browseJsonBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Select JSON Data Path", QDir::homePath(), "JSON Files (*.json)");
        if (!path.isEmpty()) jsonPathInput_->setText(path);
    });

    connect(runPdfBtn, &QPushButton::clicked, this, &ProjectDetailView::onGeneratePdf);
    connect(exportBtn, &QPushButton::clicked, this, &ProjectDetailView::onExportJson);
    connect(importBtn, &QPushButton::clicked, this, &ProjectDetailView::onImportJson);
}

void ProjectDetailView::setProject(int projectId) {
    projectId_ = projectId;
    refresh();
}

void ProjectDetailView::refresh() {
    if (projectId_ <= 0) return;

    database::ProjectRepository repo;
    auto optProj = repo.getById(projectId_);
    if (!optProj.has_value()) return;

    const auto& p = optProj.value();

    // Fill metadata headers
    projNameLabel_->setText(QString::fromStdString(p.name));
    custLabel_->setText(QString::fromStdString(p.customer));
    datesLabel_->setText(QString("Dates: %1 to %2").arg(QString::fromStdString(p.startDate), QString::fromStdString(p.endDate)));
    
    QString statusStr = QString::fromStdString(core::projectStatusToString(p.status));
    statusLabel_->setText(QString("Status: %1").arg(statusStr.toUpper()));

    // Scope tab inputs
    scopeEditor_->setPlainText(QString::fromStdString(p.scope));
    targetsEditor_->setPlainText(QString::fromStdString(p.targets));

    // Notes tab input
    notesEditor_->setPlainText(QString::fromStdString(p.notes));

    // Default file paths
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (defaultDir.isEmpty()) {
        defaultDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    pdfPathInput_->setText(QDir(defaultDir).filePath(QString("%1_report.pdf").arg(QString::fromStdString(p.name).replace(" ", "_"))));
    jsonPathInput_->setText(QDir(defaultDir).filePath(QString("%1_backup.json").arg(QString::fromStdString(p.name).replace(" ", "_"))));

    // Populate Findings log
    database::FindingRepository findRepo;
    auto findings = findRepo.getByProjectId(projectId_);
    
    findingsTable_->setRowCount(0);
    int row = 0;
    for (const auto& f : findings) {
        findingsTable_->insertRow(row);
        
        auto* idItem = new QTableWidgetItem(QString::number(f.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        findingsTable_->setItem(row, 0, idItem);
        findingsTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(f.title)));

        auto* sevItem = new QTableWidgetItem(QString::fromStdString(core::severityToString(f.severity)));
        sevItem->setTextAlignment(Qt::AlignCenter);
        
        // Colors corresponding to severity
        if (f.severity == core::Severity::Critical) sevItem->setForeground(QColor("#f87171"));
        else if (f.severity == core::Severity::High) sevItem->setForeground(QColor("#fb923c"));
        else if (f.severity == core::Severity::Medium) sevItem->setForeground(QColor("#fbbf24"));
        else if (f.severity == core::Severity::Low) sevItem->setForeground(QColor("#60a5fa"));
        else sevItem->setForeground(QColor("#94a3b8"));
        
        findingsTable_->setItem(row, 2, sevItem);

        auto* cvssItem = new QTableWidgetItem(QString::number(f.cvssScore, 'f', 1));
        cvssItem->setTextAlignment(Qt::AlignCenter);
        findingsTable_->setItem(row, 3, cvssItem);

        auto* statusItem = new QTableWidgetItem(QString::fromStdString(core::findingStatusToString(f.status)));
        statusItem->setTextAlignment(Qt::AlignCenter);
        findingsTable_->setItem(row, 4, statusItem);
        
        findingsTable_->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(f.cwe)));

        row++;
    }

    // Populate Timeline events
    database::TimelineRepository timeRepo;
    auto events = timeRepo.getByProjectId(projectId_);
    timelineList_->clear();
    for (const auto& ev : events) {
        QString text = QString("[%1]  %2").arg(QString::fromStdString(ev.timestamp), QString::fromStdString(ev.eventText));
        timelineList_->addItem(text);
    }
}

void ProjectDetailView::onSaveNotes() {
    database::ProjectRepository repo;
    auto optProj = repo.getById(projectId_);
    if (!optProj.has_value()) return;

    auto p = optProj.value();
    p.notes = notesEditor_->toPlainText().toStdString();

    if (repo.update(p)) {
        QMessageBox::information(this, "Success", "Project notes saved successfully.");
        // Log timeline event
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, "Project notes updated.");
        refresh();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save project notes.");
    }
}

void ProjectDetailView::onSaveScopeAndTargets() {
    database::ProjectRepository repo;
    auto optProj = repo.getById(projectId_);
    if (!optProj.has_value()) return;

    auto p = optProj.value();
    p.scope = scopeEditor_->toPlainText().toStdString();
    p.targets = targetsEditor_->toPlainText().toStdString();

    if (repo.update(p)) {
        QMessageBox::information(this, "Success", "Scope and Target details saved successfully.");
        // Log timeline event
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, "Scope and targets updated.");
        refresh();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save scope and targets.");
    }
}

void ProjectDetailView::onAddTimelineEvent() {
    QString text = newEventInput_->text().trimmed();
    if (text.isEmpty()) return;

    database::TimelineRepository timeRepo;
    if (timeRepo.logEvent(projectId_, text.toStdString())) {
        newEventInput_->clear();
        refresh();
    }
}

void ProjectDetailView::onGeneratePdf() {
    QString path = pdfPathInput_->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a valid output PDF path.");
        return;
    }

    database::ProjectRepository repo;
    auto optProj = repo.getById(projectId_);
    if (!optProj.has_value()) return;

    database::FindingRepository findRepo;
    auto findings = findRepo.getByProjectId(projectId_);
    auto evidenceList = findRepo.getEvidenceForProject(projectId_);

    // Run findings validation check before generation
    auto warnings = services::PdfGenerator::validateFindings(findings, evidenceList);
    if (!warnings.empty()) {
        QString warningMsg = "<b>Professional Report Validation Warning</b><br><br>"
                             "The following findings are missing required professional fields:<br><br>";
        for (const auto& w : warnings) {
            warningMsg += QString("• %1<br>").arg(QString::fromStdString(w));
        }
        warningMsg += "<br>Do you want to proceed and generate the report anyway?";

        auto result = QMessageBox::warning(this, "Incomplete Report Validation", warningMsg,
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (result == QMessageBox::No) {
            return; // Abort!
        }
    }

    // Generate PDF using service
    if (services::PdfGenerator::generateReport(path.toStdString(), optProj.value(), findings, evidenceList)) {
        QMessageBox::information(this, "Success", "PDF report generated successfully!");
        
        // Log timeline event
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, "PDF report generated: " + QFileInfo(path).fileName().toStdString());
        refresh();
    } else {
        QMessageBox::critical(this, "Error", "Failed to generate PDF report.");
    }
}

void ProjectDetailView::onExportJson() {
    QString path = jsonPathInput_->text().trimmed();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a valid JSON output path.");
        return;
    }

    database::ProjectRepository repo;
    auto optProj = repo.getById(projectId_);
    if (!optProj.has_value()) return;

    database::FindingRepository findRepo;
    auto findings = findRepo.getByProjectId(projectId_);

    if (services::ImportExportService::exportProjectJson(path.toStdString(), optProj.value(), findings)) {
        QMessageBox::information(this, "Success", "Project exported to JSON successfully.");
        // Log timeline event
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, "Project data backed up to JSON.");
        refresh();
    } else {
        QMessageBox::critical(this, "Error", "Failed to export project to JSON.");
    }
}

void ProjectDetailView::onImportJson() {
    QString path = QFileDialog::getOpenFileName(this, "Select JSON Data to Import", QDir::currentPath(), "JSON Files (*.json)");
    if (path.isEmpty()) return;

    models::Project importedProj;
    std::vector<models::Finding> importedFindings;

    if (services::ImportExportService::importProjectJson(path.toStdString(), importedProj, importedFindings)) {
        // Merge data into current project
        database::ProjectRepository repo;
        auto optProj = repo.getById(projectId_);
        if (!optProj.has_value()) return;

        auto p = optProj.value();
        // Update metadata from imported if desired, but here we just merge findings
        database::FindingRepository findRepo;
        int importCount = 0;
        for (auto& f : importedFindings) {
            f.projectId = projectId_; // assign current project id
            if (findRepo.insert(f)) {
                importCount++;
            }
        }

        QMessageBox::information(this, "Success", QString("Imported and merged %1 findings into the current project.").arg(importCount));
        
        // Log timeline event
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, QString("Imported %1 findings from JSON.").arg(importCount).toStdString());
        refresh();
    } else {
        QMessageBox::critical(this, "Error", "Failed to import JSON data.");
    }
}

void ProjectDetailView::onFindingDoubleClicked(int row, int) {
    auto* idItem = findingsTable_->item(row, 0);
    if (idItem) {
        int id = idItem->text().toInt();
        emit editFindingRequested(id);
    }
}

void ProjectDetailView::onDeleteProject() {
    auto res = QMessageBox::question(this, "Confirm Delete", 
        "Are you absolutely sure you want to delete this project and all its findings?\nThis action cannot be undone.", 
        QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        database::ProjectRepository repo;
        if (repo.remove(projectId_)) {
            emit backRequested();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete project.");
        }
    }
}

} // namespace reportforge::widgets
