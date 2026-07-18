#include "template_dialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

namespace reportforge::dialogs {

TemplateDialog::TemplateDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Vulnerability Template Editor");
    resize(500, 480);
    setStyleSheet("QDialog { background-color: #111115; color: #e2e8f0; } QLabel { color: #94a3b8; }");

    auto* mainLayout = new QVBoxLayout(this);
    
    auto* formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(10);

    titleInput_ = new QLineEdit(this);
    titleInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("Template Title:", titleInput_);

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

    cweInput_ = new QLineEdit(this);
    cweInput_->setPlaceholderText("CWE-89");
    cweInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("CWE Reference:", cweInput_);

    owaspInput_ = new QLineEdit(this);
    owaspInput_->setPlaceholderText("A03:2021-Injection");
    owaspInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    formLayout->addRow("OWASP Category:", owaspInput_);

    descInput_ = new QTextEdit(this);
    descInput_->setStyleSheet("QTextEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    descInput_->setMaximumHeight(70);
    formLayout->addRow("Description:", descInput_);

    impactInput_ = new QTextEdit(this);
    impactInput_->setStyleSheet("QTextEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    impactInput_->setMaximumHeight(70);
    formLayout->addRow("Impact:", impactInput_);

    recInput_ = new QTextEdit(this);
    recInput_->setStyleSheet("QTextEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 5px; border-radius: 4px; }");
    recInput_->setMaximumHeight(70);
    formLayout->addRow("Recommendation:", recInput_);

    mainLayout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("background-color: #8b5cf6; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-weight: bold;");
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet("background-color: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px 12px; border-radius: 4px;");
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void TemplateDialog::setTemplate(const models::FindingTemplate& t) {
    id_ = t.id;
    titleInput_->setText(QString::fromStdString(t.title));
    severityInput_->setCurrentText(QString::fromStdString(core::severityToString(t.severity)));
    cvssInput_->setValue(t.cvssScore);
    cweInput_->setText(QString::fromStdString(t.cwe));
    owaspInput_->setText(QString::fromStdString(t.owaspCategory));
    descInput_->setPlainText(QString::fromStdString(t.description));
    impactInput_->setPlainText(QString::fromStdString(t.impact));
    recInput_->setPlainText(QString::fromStdString(t.recommendation));
}

models::FindingTemplate TemplateDialog::getTemplate() const {
    models::FindingTemplate t;
    t.id = id_;
    t.title = titleInput_->text().trimmed().toStdString();
    t.severity = core::stringToSeverity(severityInput_->currentText().toStdString());
    t.cvssScore = cvssInput_->value();
    t.cwe = cweInput_->text().trimmed().toStdString();
    t.owaspCategory = owaspInput_->text().trimmed().toStdString();
    t.description = descInput_->toPlainText().trimmed().toStdString();
    t.impact = impactInput_->toPlainText().trimmed().toStdString();
    t.recommendation = recInput_->toPlainText().trimmed().toStdString();
    return t;
}

} // namespace reportforge::dialogs
