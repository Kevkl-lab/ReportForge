#include "finding_detail_view.h"
#include "../database/finding_repository.h"
#include "../database/template_repository.h"
#include "../database/timeline_repository.h"
#include "../services/cvss_calculator.h"
#include "../utils/file_utils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QFileInfo>
#include <QVariant>
#include <QDebug>

namespace reportforge::widgets {

FindingDetailView::FindingDetailView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(15);

    // Header bar
    auto* header = new QHBoxLayout();
    auto* backBtn = new QPushButton("< Back", this);
    backBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #94a3b8;"
        "  border: 1px solid #2d2d34;"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "}"
        "QPushButton:hover { color: #a78bfa; border-color: #8b5cf6; }"
    );
    header->addWidget(backBtn);
    header->addStretch();

    auto* saveBtn = new QPushButton("Save Finding", this);
    saveBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    header->addWidget(saveBtn);
    mainLayout->addLayout(header);

    // Editor columns layout
    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Left Column: Metadata & CVSS Calculator
    auto* leftCol = new QWidget(this);
    leftCol->setFixedWidth(320);
    auto* leftLayout = new QVBoxLayout(leftCol);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(15);

    // Meta Group Box
    auto* metaGroup = new QGroupBox("Finding Details", this);
    metaGroup->setStyleSheet("QGroupBox { color: #a78bfa; font-weight: bold; border: 1px solid #2d2d34; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; }");
    auto* metaForm = new QFormLayout(metaGroup);
    metaForm->setVerticalSpacing(10);

    titleInput_ = new QLineEdit(metaGroup);
    titleInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; } QLineEdit:focus { border-color: #8b5cf6; }");
    metaForm->addRow("Title:", titleInput_);

    severityInput_ = new QComboBox(metaGroup);
    severityInput_->addItems({"Critical", "High", "Medium", "Low", "Informational"});
    severityInput_->setStyleSheet("QComboBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("Severity:", severityInput_);

    cvssScoreInput_ = new QDoubleSpinBox(metaGroup);
    cvssScoreInput_->setRange(0.0, 10.0);
    cvssScoreInput_->setSingleStep(0.1);
    cvssScoreInput_->setDecimals(1);
    cvssScoreInput_->setStyleSheet("QDoubleSpinBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("CVSS Base Score:", cvssScoreInput_);

    statusInput_ = new QComboBox(metaGroup);
    statusInput_->addItems({"Open", "Fixed", "Retested", "Accepted Risk"});
    statusInput_->setStyleSheet("QComboBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("Status:", statusInput_);

    cweInput_ = new QLineEdit(metaGroup);
    cweInput_->setPlaceholderText("e.g. CWE-79");
    cweInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("CWE ID:", cweInput_);

    owaspInput_ = new QLineEdit(metaGroup);
    owaspInput_->setPlaceholderText("e.g. A03:2021-Injection");
    owaspInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("OWASP Category:", owaspInput_);

    affectedAssetsInput_ = new QLineEdit(metaGroup);
    affectedAssetsInput_->setPlaceholderText("e.g. https://api.domain.com/login");
    affectedAssetsInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("Affected Assets:", affectedAssetsInput_);

    tagsInput_ = new QLineEdit(metaGroup);
    tagsInput_->setPlaceholderText("e.g. web, api, injection");
    tagsInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("Tags:", tagsInput_);

    templateSelect_ = new QComboBox(metaGroup);
    templateSelect_->setStyleSheet("QComboBox { background: #16161e; color: #a78bfa; border: 1px solid #8b5cf6; padding: 6px; border-radius: 4px; }");
    metaForm->addRow("Load Template:", templateSelect_);

    leftLayout->addWidget(metaGroup);

    // CVSS Calculator Group Box
    auto* cvssGroup = new QGroupBox("CVSS v3.1 Calculator", this);
    cvssGroup->setStyleSheet("QGroupBox { color: #a78bfa; font-weight: bold; border: 1px solid #2d2d34; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; }");
    auto* cvssForm = new QFormLayout(cvssGroup);
    cvssForm->setVerticalSpacing(6);

    avCombo_ = new QComboBox(cvssGroup);
    avCombo_->addItems({"Network", "Adjacent", "Local", "Physical"});
    cvssForm->addRow("Attack Vector:", avCombo_);

    acCombo_ = new QComboBox(cvssGroup);
    acCombo_->addItems({"Low", "High"});
    cvssForm->addRow("Complexity:", acCombo_);

    prCombo_ = new QComboBox(cvssGroup);
    prCombo_->addItems({"None", "Low", "High"});
    cvssForm->addRow("Privileges Req:", prCombo_);

    uiCombo_ = new QComboBox(cvssGroup);
    uiCombo_->addItems({"None", "Required"});
    cvssForm->addRow("User Interaction:", uiCombo_);

    sCombo_ = new QComboBox(cvssGroup);
    sCombo_->addItems({"Unchanged", "Changed"});
    cvssForm->addRow("Scope:", sCombo_);

    cCombo_ = new QComboBox(cvssGroup);
    cCombo_->addItems({"High", "Low", "None"});
    cvssForm->addRow("Confidentiality:", cCombo_);

    iCombo_ = new QComboBox(cvssGroup);
    iCombo_->addItems({"High", "Low", "None"});
    cvssForm->addRow("Integrity:", iCombo_);

    aCombo_ = new QComboBox(cvssGroup);
    aCombo_->addItems({"High", "Low", "None"});
    cvssForm->addRow("Availability:", aCombo_);

    cvssVectorLabel_ = new QLabel("CVSS:3.1/AV:N/AC:L/PR:N/UI:N/S:U/C:H/I:H/A:H", cvssGroup);
    cvssVectorLabel_->setWordWrap(true);
    cvssVectorLabel_->setStyleSheet("color: #64748b; font-size: 9px; font-family: monospace;");
    cvssForm->addRow("Vector:", cvssVectorLabel_);

    leftLayout->addWidget(cvssGroup);
    contentLayout->addWidget(leftCol);

    // Right Column: Tabbed Text Editors & Evidence Upload
    auto* rightCol = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightCol);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    textTabs_ = new QTabWidget(this);
    textTabs_->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #2d2d34; background: #16161e; border-radius: 6px; }"
        "QTabBar::tab { background: #232329; color: #94a3b8; padding: 6px 12px; margin-right: 2px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:selected { background: #16161e; color: #a78bfa; border-bottom: 2px solid #8b5cf6; }"
    );

    descEditor_ = new MarkdownEditor(this);
    descEditor_->setPlaceholderText("Detailed description of the vulnerability...");
    textTabs_->addTab(descEditor_, "Description");

    impactEditor_ = new MarkdownEditor(this);
    impactEditor_->setPlaceholderText("Explain business/security impact of exploitation...");
    textTabs_->addTab(impactEditor_, "Impact");

    recEditor_ = new MarkdownEditor(this);
    recEditor_->setPlaceholderText("Mitigation or remediation recommendations...");
    textTabs_->addTab(recEditor_, "Recommendation");

    pocEditor_ = new MarkdownEditor(this);
    pocEditor_->setPlaceholderText("Monospace payloads, exploit code scripts, or HTTP requests...");
    textTabs_->addTab(pocEditor_, "Proof of Concept");

    stepsEditor_ = new MarkdownEditor(this);
    stepsEditor_->setPlaceholderText("1. Navigate to page\n2. Inject payload...");
    textTabs_->addTab(stepsEditor_, "Reproduction Steps");

    rightLayout->addWidget(textTabs_, 2);

    // Evidence upload area
    auto* evidenceGroup = new QGroupBox("Evidence Media Attachments", this);
    evidenceGroup->setStyleSheet("QGroupBox { color: #a78bfa; font-weight: bold; border: 1px solid #2d2d34; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 15px; }");
    auto* evidenceLayout = new QHBoxLayout(evidenceGroup);
    
    dragDropArea_ = new DragDropArea(evidenceGroup);
    evidenceLayout->addWidget(dragDropArea_, 1);

    auto* listLayout = new QVBoxLayout();
    evidenceList_ = new QListWidget(evidenceGroup);
    evidenceList_->setMinimumHeight(80);
    evidenceList_->setStyleSheet("QListWidget { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; border-radius: 6px; }");
    listLayout->addWidget(evidenceList_);

    auto* deleteEvBtn = new QPushButton("Remove Attachment", evidenceGroup);
    deleteEvBtn->setStyleSheet("QPushButton { background: #232329; color: #f87171; border: 1px solid #991b1b; padding: 4px 8px; border-radius: 4px; } QPushButton:hover { background: #7f1d1d; }");
    listLayout->addWidget(deleteEvBtn);

    evidenceLayout->addLayout(listLayout, 1);
    rightLayout->addWidget(evidenceGroup, 1);

    contentLayout->addWidget(rightCol, 1);
    mainLayout->addLayout(contentLayout, 1);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, [this]() { emit backRequested(projectId_); });
    connect(saveBtn, &QPushButton::clicked, this, &FindingDetailView::onSave);
    connect(dragDropArea_, &DragDropArea::filesDropped, this, &FindingDetailView::onFilesDropped);
    connect(deleteEvBtn, &QPushButton::clicked, this, &FindingDetailView::onDeleteEvidence);

    // CVSS calculator signals
    connect(avCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(acCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(prCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(uiCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(sCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(cCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(iCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);
    connect(aCombo_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onCvssMetricChanged);

    // Template apply signal
    connect(templateSelect_, &QComboBox::currentIndexChanged, this, &FindingDetailView::onApplyTemplate);
}

void FindingDetailView::setFinding(int findingId, int projectId) {
    findingId_ = findingId;
    projectId_ = projectId;
    refresh();
}

void FindingDetailView::refresh() {
    // Populate templates dropdown
    templateSelect_->blockSignals(true);
    templateSelect_->clear();
    templateSelect_->addItem("-- Select a Template --", 0);
    
    database::TemplateRepository tempRepo;
    auto templates = tempRepo.getAll();
    for (const auto& t : templates) {
        templateSelect_->addItem(QString::fromStdString(t.title), t.id);
    }
    templateSelect_->blockSignals(false);

    if (findingId_ <= 0) {
        // Create mode
        titleInput_->clear();
        severityInput_->setCurrentIndex(2); // Medium
        cvssScoreInput_->setValue(5.0);
        statusInput_->setCurrentIndex(0); // Open
        cweInput_->clear();
        owaspInput_->clear();
        affectedAssetsInput_->clear();
        tagsInput_->clear();
        descEditor_->setPlainText("");
        impactEditor_->setPlainText("");
        recEditor_->setPlainText("");
        pocEditor_->setPlainText("");
        stepsEditor_->setPlainText("");
        evidenceList_->clear();

        // Default CVSS calculator metrics
        avCombo_->setCurrentIndex(0);
        acCombo_->setCurrentIndex(0);
        prCombo_->setCurrentIndex(0);
        uiCombo_->setCurrentIndex(0);
        sCombo_->setCurrentIndex(0);
        cCombo_->setCurrentIndex(0);
        iCombo_->setCurrentIndex(0);
        aCombo_->setCurrentIndex(0);
        onCvssMetricChanged();
        return;
    }

    // Edit mode
    database::FindingRepository findRepo;
    auto optF = findRepo.getById(findingId_);
    if (!optF.has_value()) return;

    const auto& f = optF.value();
    projectId_ = f.projectId;

    titleInput_->setText(QString::fromStdString(f.title));
    
    QString sevStr = QString::fromStdString(core::severityToString(f.severity));
    severityInput_->setCurrentText(sevStr);
    
    cvssScoreInput_->setValue(f.cvssScore);
    
    QString statusStr = QString::fromStdString(core::findingStatusToString(f.status));
    statusInput_->setCurrentText(statusStr);
    
    cweInput_->setText(QString::fromStdString(f.cwe));
    owaspInput_->setText(QString::fromStdString(f.owaspCategory));
    affectedAssetsInput_->setText(QString::fromStdString(f.affectedAssets));
    tagsInput_->setText(QString::fromStdString(f.tags));

    descEditor_->setPlainText(QString::fromStdString(f.description));
    impactEditor_->setPlainText(QString::fromStdString(f.impact));
    recEditor_->setPlainText(QString::fromStdString(f.recommendation));
    pocEditor_->setPlainText(QString::fromStdString(f.proofOfConcept));
    stepsEditor_->setPlainText(QString::fromStdString(f.reproductionSteps));

    // Populate evidence list
    evidenceList_->clear();
    auto evidence = findRepo.getEvidenceForFinding(findingId_);
    for (const auto& ev : evidence) {
        auto* item = new QListWidgetItem(evidenceList_);
        item->setText(QString::fromStdString(ev.fileName));
        item->setData(Qt::UserRole, ev.id);
        evidenceList_->addItem(item);
    }
}

void FindingDetailView::onSave() {
    QString title = titleInput_->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a finding title.");
        return;
    }

    database::FindingRepository findRepo;
    models::Finding f;
    f.id = findingId_;
    f.projectId = projectId_;
    f.title = title.toStdString();
    f.severity = core::stringToSeverity(severityInput_->currentText().toStdString());
    f.cvssScore = cvssScoreInput_->value();
    f.status = core::stringToFindingStatus(statusInput_->currentText().toStdString());
    f.cwe = cweInput_->text().trimmed().toStdString();
    f.owaspCategory = owaspInput_->text().trimmed().toStdString();
    f.affectedAssets = affectedAssetsInput_->text().trimmed().toStdString();
    f.tags = tagsInput_->text().trimmed().toStdString();

    f.description = descEditor_->toPlainText().toStdString();
    f.impact = impactEditor_->toPlainText().toStdString();
    f.recommendation = recEditor_->toPlainText().toStdString();
    f.proofOfConcept = pocEditor_->toPlainText().toStdString();
    f.reproductionSteps = stepsEditor_->toPlainText().toStdString();

    bool success = false;
    if (findingId_ <= 0) {
        success = findRepo.insert(f);
        if (success) {
            database::TimelineRepository timeRepo;
            timeRepo.logEvent(projectId_, "Vulnerability logged: " + f.title);
        }
    } else {
        success = findRepo.update(f);
    }

    if (success) {
        QMessageBox::information(this, "Success", "Finding saved successfully.");
        emit backRequested(projectId_);
    } else {
        QMessageBox::critical(this, "Error", "Failed to save finding.");
    }
}

void FindingDetailView::onApplyTemplate(int index) {
    if (index <= 0) return;

    int tempId = templateSelect_->itemData(index).toInt();
    database::TemplateRepository tempRepo;
    auto optT = tempRepo.getById(tempId);
    if (!optT.has_value()) return;

    const auto& t = optT.value();

    titleInput_->setText(QString::fromStdString(t.title));
    severityInput_->setCurrentText(QString::fromStdString(core::severityToString(t.severity)));
    cvssScoreInput_->setValue(t.cvssScore);
    cweInput_->setText(QString::fromStdString(t.cwe));
    owaspInput_->setText(QString::fromStdString(t.owaspCategory));
    
    descEditor_->setPlainText(QString::fromStdString(t.description));
    impactEditor_->setPlainText(QString::fromStdString(t.impact));
    recEditor_->setPlainText(QString::fromStdString(t.recommendation));
    pocEditor_->setPlainText("");
    stepsEditor_->setPlainText("");

    // Reset template selection box back to placeholder
    templateSelect_->blockSignals(true);
    templateSelect_->setCurrentIndex(0);
    templateSelect_->blockSignals(false);
}

void FindingDetailView::onFilesDropped(const QStringList& filePaths) {
    if (findingId_ <= 0) {
        QMessageBox::warning(this, "Save Required", "Please save this finding first before uploading evidence files.");
        return;
    }

    database::FindingRepository findRepo;
    int uploadCount = 0;
    for (const auto& path : filePaths) {
        QFileInfo fi(path);
        std::string copiedPath = utils::FileUtils::copyEvidenceFile(projectId_, path.toStdString(), fi.fileName().toStdString());
        if (!copiedPath.empty()) {
            models::Evidence ev;
            ev.projectId = projectId_;
            ev.findingId = findingId_;
            ev.filePath = copiedPath;
            ev.fileName = fi.fileName().toStdString();
            
            // Map extensions
            QString ext = fi.suffix().toLower();
            if (ext == "png" || ext == "jpg" || ext == "jpeg") {
                ev.fileType = core::MediaType::Image;
            } else if (ext == "pdf") {
                ev.fileType = core::MediaType::PDF;
            } else if (ext == "log") {
                ev.fileType = core::MediaType::Log;
            } else if (ext == "txt") {
                ev.fileType = core::MediaType::Text;
            } else {
                ev.fileType = core::MediaType::Other;
            }

            if (findRepo.addEvidence(ev)) {
                uploadCount++;
            }
        }
    }

    if (uploadCount > 0) {
        database::TimelineRepository timeRepo;
        timeRepo.logEvent(projectId_, QString("Uploaded %1 evidence files to finding: %2").arg(uploadCount).arg(titleInput_->text()).toStdString());
        refresh();
    }
}

void FindingDetailView::onDeleteEvidence() {
    int row = evidenceList_->currentRow();
    if (row < 0) return;

    auto* item = evidenceList_->item(row);
    if (!item) return;

    int evId = item->data(Qt::UserRole).toInt();

    auto res = QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this evidence attachment?", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        database::FindingRepository findRepo;
        if (findRepo.removeEvidence(evId)) {
            refresh();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete evidence attachment.");
        }
    }
}

void FindingDetailView::onCvssMetricChanged() {
    services::CvssCalculator::Metrics m;
    m.attackVector = avCombo_->currentIndex();
    m.attackComplexity = acCombo_->currentIndex();
    m.privilegesRequired = prCombo_->currentIndex();
    m.userInteraction = uiCombo_->currentIndex();
    m.scope = sCombo_->currentIndex();
    m.confidentiality = cCombo_->currentIndex();
    m.integrity = iCombo_->currentIndex();
    m.availability = aCombo_->currentIndex();

    double score = services::CvssCalculator::calculate(m);
    cvssScoreInput_->setValue(score);

    std::string vector = services::CvssCalculator::toStringVector(m);
    cvssVectorLabel_->setText(QString::fromStdString(vector));

    // Proactively align Severity dropdown with CVSS score
    if (score >= 9.0) {
        severityInput_->setCurrentText("Critical");
    } else if (score >= 7.0) {
        severityInput_->setCurrentText("High");
    } else if (score >= 4.0) {
        severityInput_->setCurrentText("Medium");
    } else if (score > 0.0) {
        severityInput_->setCurrentText("Low");
    } else {
        severityInput_->setCurrentText("Informational");
    }
}

} // namespace reportforge::widgets
