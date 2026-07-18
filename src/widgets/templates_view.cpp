#include "templates_view.h"
#include "../database/template_repository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QVariant>
#include <QDebug>

namespace reportforge::widgets {

TemplatesView::TemplatesView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Header row
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("VULNERABILITY TEMPLATES", this);
    titleLabel->setStyleSheet("color: #e2e8f0; font-size: 20px; font-weight: bold; letter-spacing: 1px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    addTemplateBtn_ = new QPushButton("+ New Template", this);
    addTemplateBtn_->setMinimumHeight(35);
    addTemplateBtn_->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 0 16px;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #a78bfa; }"
    );
    headerLayout->addWidget(addTemplateBtn_);

    deleteTemplateBtn_ = new QPushButton("Delete Selected", this);
    deleteTemplateBtn_->setMinimumHeight(35);
    deleteTemplateBtn_->setStyleSheet(
        "QPushButton {"
        "  background-color: #232329;"
        "  color: #f87171;"
        "  border: 1px solid #991b1b;"
        "  border-radius: 6px;"
        "  padding: 0 16px;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #7f1d1d; }"
    );
    headerLayout->addWidget(deleteTemplateBtn_);
    
    mainLayout->addLayout(headerLayout);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({"ID", "Template Title", "Severity", "CVSS Score", "CWE", "OWASP Category"});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    
    // Style headers
    table_->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "  background-color: #111115;"
        "  color: #a78bfa;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 10px;"
        "  border: none;"
        "  border-bottom: 2px solid #23232f;"
        "}"
    );
    
    // Style body
    table_->setStyleSheet(
        "QTableWidget {"
        "  background-color: #16161e;"
        "  alternate-background-color: #1a1a24;"
        "  color: #e2e8f0;"
        "  gridline-color: #1f1f2a;"
        "  border: 1px solid #23232f;"
        "  border-radius: 8px;"
        "}"
        "QTableWidget::item { padding: 10px; }"
        "QTableWidget::item:selected { background-color: #232332; color: #a78bfa; }"
    );
    
    table_->setAlternatingRowColors(true);
    mainLayout->addWidget(table_);

    // Connections
    connect(addTemplateBtn_, &QPushButton::clicked, this, &TemplatesView::createTemplateRequested);
    connect(deleteTemplateBtn_, &QPushButton::clicked, this, &TemplatesView::onDeleteTemplate);
    connect(table_, &QTableWidget::cellDoubleClicked, this, &TemplatesView::onRowDoubleClicked);
}

void TemplatesView::refresh() {
    database::TemplateRepository repo;
    auto templates = repo.getAll();

    table_->setRowCount(0);
    int row = 0;
    for (const auto& t : templates) {
        table_->insertRow(row);

        auto* idItem = new QTableWidgetItem(QString::number(t.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 0, idItem);

        table_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(t.title)));

        auto* sevItem = new QTableWidgetItem(QString::fromStdString(core::severityToString(t.severity)));
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (t.severity == core::Severity::Critical) sevItem->setForeground(QColor("#f87171"));
        else if (t.severity == core::Severity::High) sevItem->setForeground(QColor("#fb923c"));
        else if (t.severity == core::Severity::Medium) sevItem->setForeground(QColor("#fbbf24"));
        else if (t.severity == core::Severity::Low) sevItem->setForeground(QColor("#60a5fa"));
        else sevItem->setForeground(QColor("#94a3b8"));
        table_->setItem(row, 2, sevItem);

        auto* cvssItem = new QTableWidgetItem(QString::number(t.cvssScore, 'f', 1));
        cvssItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 3, cvssItem);

        table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(t.cwe)));
        table_->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(t.owaspCategory)));

        row++;
    }
}

void TemplatesView::onRowDoubleClicked(int row, int) {
    auto* idItem = table_->item(row, 0);
    if (idItem) {
        int id = idItem->text().toInt();
        emit editTemplateRequested(id);
    }
}

void TemplatesView::onDeleteTemplate() {
    int row = table_->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Please select a template to delete.");
        return;
    }

    auto* idItem = table_->item(row, 0);
    if (!idItem) return;
    int id = idItem->text().toInt();

    auto res = QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this template?", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        database::TemplateRepository repo;
        if (repo.remove(id)) {
            refresh();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete template.");
        }
    }
}

} // namespace reportforge::widgets
