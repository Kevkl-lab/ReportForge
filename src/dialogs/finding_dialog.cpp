#include "finding_dialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

namespace reportforge::dialogs {

FindingDialog::FindingDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Vulnerability Metadata Editor");
    resize(420, 360);
    setStyleSheet("QDialog { background-color: #111115; color: #e2e8f0; } QLabel { color: #94a3b8; }");

    auto* mainLayout = new QVBoxLayout(this);
    
    auto* formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(10);

    titleInput_ = new QLineEdit(this);
    titleInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Finding Title:", titleInput_);

    severityInput_ = new QComboBox(this);
    severityInput_->addItems({"Critical", "High", "Medium", "Low", "Informational"});
    severityInput_->setStyleSheet("QComboBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Severity Rating:", severityInput_);

    cvssInput_ = new QDoubleSpinBox(this);
    cvssInput_->setRange(0.0, 10.0);
    cvssInput_->setSingleStep(0.1);
    cvssInput_->setDecimals(1);
    cvssInput_->setStyleSheet("QDoubleSpinBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("CVSS Base Score:", cvssInput_);

    statusInput_ = new QComboBox(this);
    statusInput_->addItems({"Open", "Fixed", "Retested", "Accepted Risk"});
    statusInput_->setStyleSheet("QComboBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Testing Status:", statusInput_);

    cweInput_ = new QLineEdit(this);
    cweInput_->setPlaceholderText("CWE-79");
    cweInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("CWE Reference:", cweInput_);

    owaspInput_ = new QLineEdit(this);
    owaspInput_->setPlaceholderText("A03:2021-Injection");
    owaspInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("OWASP Category:", owaspInput_);

    assetsInput_ = new QLineEdit(this);
    assetsInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Affected Assets:", assetsInput_);

    tagsInput_ = new QLineEdit(this);
    tagsInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Tags:", tagsInput_);

    mainLayout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("background-color: #8b5cf6; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-weight: bold;");
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet("background-color: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px 12px; border-radius: 4px;");
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void FindingDialog::setFinding(const models::Finding& f) {
    id_ = f.id;
    projectId_ = f.projectId;
    titleInput_->setText(QString::fromStdString(f.title));
    severityInput_->setCurrentText(QString::fromStdString(core::severityToString(f.severity)));
    cvssInput_->setValue(f.cvssScore);
    statusInput_->setCurrentText(QString::fromStdString(core::findingStatusToString(f.status)));
    cweInput_->setText(QString::fromStdString(f.cwe));
    owaspInput_->setText(QString::fromStdString(f.owaspCategory));
    assetsInput_->setText(QString::fromStdString(f.affectedAssets));
    tagsInput_->setText(QString::fromStdString(f.tags));

    // Preserve rich content fields
    desc_ = f.description;
    impact_ = f.impact;
    rec_ = f.recommendation;
    poc_ = f.proofOfConcept;
    steps_ = f.reproductionSteps;
    evidence_ = f.evidence;
}

models::Finding FindingDialog::getFinding() const {
    models::Finding f;
    f.id = id_;
    f.projectId = projectId_;
    f.title = titleInput_->text().trimmed().toStdString();
    f.severity = core::stringToSeverity(severityInput_->currentText().toStdString());
    f.cvssScore = cvssInput_->value();
    f.status = core::stringToFindingStatus(statusInput_->currentText().toStdString());
    f.cwe = cweInput_->text().trimmed().toStdString();
    f.owaspCategory = owaspInput_->text().trimmed().toStdString();
    f.affectedAssets = assetsInput_->text().trimmed().toStdString();
    f.tags = tagsInput_->text().trimmed().toStdString();
    
    // Restore preserved content fields
    f.description = desc_;
    f.impact = impact_;
    f.recommendation = rec_;
    f.proofOfConcept = poc_;
    f.reproductionSteps = steps_;
    f.evidence = evidence_;
    return f;
}

} // namespace reportforge::dialogs
