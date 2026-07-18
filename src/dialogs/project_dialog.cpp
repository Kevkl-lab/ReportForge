#include "project_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDate>
#include <QPushButton>

namespace reportforge::dialogs {

ProjectDialog::ProjectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Project Configuration");
    resize(400, 320);
    setStyleSheet("QDialog { background-color: #111115; color: #e2e8f0; } QLabel { color: #94a3b8; }");

    auto* mainLayout = new QVBoxLayout(this);
    
    auto* formLayout = new QFormLayout();
    formLayout->setVerticalSpacing(12);

    customerInput_ = new QLineEdit(this);
    customerInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    formLayout->addRow("Customer Name:", customerInput_);

    nameInput_ = new QLineEdit(this);
    nameInput_->setStyleSheet("QLineEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    formLayout->addRow("Project Name:", nameInput_);

    descInput_ = new QTextEdit(this);
    descInput_->setStyleSheet("QTextEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    descInput_->setMaximumHeight(80);
    formLayout->addRow("Description:", descInput_);

    startInput_ = new QDateEdit(this);
    startInput_->setCalendarPopup(true);
    startInput_->setDate(QDate::currentDate());
    startInput_->setStyleSheet("QDateEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    formLayout->addRow("Start Date:", startInput_);

    endInput_ = new QDateEdit(this);
    endInput_->setCalendarPopup(true);
    endInput_->setDate(QDate::currentDate().addDays(7));
    endInput_->setStyleSheet("QDateEdit { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    formLayout->addRow("End Date:", endInput_);

    statusInput_ = new QComboBox(this);
    statusInput_->addItems({"Planning", "Testing", "Retesting", "Completed"});
    statusInput_->setStyleSheet("QComboBox { background: #16161e; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px; border-radius: 4px; }");
    formLayout->addRow("Project Status:", statusInput_);

    mainLayout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("background-color: #8b5cf6; color: white; border: none; padding: 6px 12px; border-radius: 4px; font-weight: bold;");
    buttonBox->button(QDialogButtonBox::Cancel)->setStyleSheet("background-color: #232329; color: #e2e8f0; border: 1px solid #2d2d34; padding: 6px 12px; border-radius: 4px;");
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ProjectDialog::setProject(const models::Project& p) {
    projectId_ = p.id;
    customerInput_->setText(QString::fromStdString(p.customer));
    nameInput_->setText(QString::fromStdString(p.name));
    descInput_->setPlainText(QString::fromStdString(p.description));
    
    QDate sDate = QDate::fromString(QString::fromStdString(p.startDate), "yyyy-MM-dd");
    if (sDate.isValid()) startInput_->setDate(sDate);
    
    QDate eDate = QDate::fromString(QString::fromStdString(p.endDate), "yyyy-MM-dd");
    if (eDate.isValid()) endInput_->setDate(eDate);

    statusInput_->setCurrentText(QString::fromStdString(core::projectStatusToString(p.status)));

    // Preserve non-metadata fields
    scope_ = p.scope;
    targets_ = p.targets;
    notes_ = p.notes;
}

models::Project ProjectDialog::getProject() const {
    models::Project p;
    p.id = projectId_;
    p.customer = customerInput_->text().trimmed().toStdString();
    p.name = nameInput_->text().trimmed().toStdString();
    p.description = descInput_->toPlainText().trimmed().toStdString();
    p.startDate = startInput_->date().toString("yyyy-MM-dd").toStdString();
    p.endDate = endInput_->date().toString("yyyy-MM-dd").toStdString();
    p.status = core::stringToProjectStatus(statusInput_->currentText().toStdString());
    p.scope = scope_;
    p.targets = targets_;
    p.notes = notes_;
    return p;
}

} // namespace reportforge::dialogs
