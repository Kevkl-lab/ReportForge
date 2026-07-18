#include "projects_view.h"
#include "../database/project_repository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVariant>
#include <QDebug>

namespace reportforge::widgets {

ProjectsView::ProjectsView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Header row
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("PROJECTS", this);
    titleLabel->setStyleSheet("color: #e2e8f0; font-size: 20px; font-weight: bold; letter-spacing: 1px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    newProjectBtn_ = new QPushButton("+ New Project", this);
    newProjectBtn_->setMinimumHeight(35);
    newProjectBtn_->setStyleSheet(
        "QPushButton {"
        "  background-color: #8b5cf6;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 0 16px;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #a78bfa;"
        "}"
    );
    headerLayout->addWidget(newProjectBtn_);
    mainLayout->addLayout(headerLayout);

    // Filters row
    auto* filtersLayout = new QHBoxLayout();
    filtersLayout->setSpacing(15);

    searchBar_ = new QLineEdit(this);
    searchBar_->setPlaceholderText("Search by Customer or Project Name...");
    searchBar_->setMinimumHeight(35);
    searchBar_->setStyleSheet(
        "QLineEdit {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "  padding-left: 12px;"
        "  font-size: 12px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #8b5cf6;"
        "}"
    );
    filtersLayout->addWidget(searchBar_, 1);

    statusFilter_ = new QComboBox(this);
    statusFilter_->addItem("All Statuses");
    statusFilter_->addItem("Planning");
    statusFilter_->addItem("Testing");
    statusFilter_->addItem("Retesting");
    statusFilter_->addItem("Completed");
    statusFilter_->setMinimumHeight(35);
    statusFilter_->setFixedWidth(150);
    statusFilter_->setStyleSheet(
        "QComboBox {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "  padding-left: 10px;"
        "  font-size: 12px;"
        "}"
        "QComboBox:focus {"
        "  border: 1px solid #8b5cf6;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  selection-background-color: #8b5cf6;"
        "}"
    );
    filtersLayout->addWidget(statusFilter_);
    mainLayout->addLayout(filtersLayout);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({"ID", "Customer", "Project Name", "Start Date", "End Date", "Status"});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    
    // Style table headers
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
    
    // Style table body
    table_->setStyleSheet(
        "QTableWidget {"
        "  background-color: #16161e;"
        "  alternate-background-color: #1a1a24;"
        "  color: #e2e8f0;"
        "  gridline-color: #1f1f2a;"
        "  border: 1px solid #23232f;"
        "  border-radius: 8px;"
        "}"
        "QTableWidget::item {"
        "  padding: 10px;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #232332;"
        "  color: #a78bfa;"
        "}"
    );
    
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    
    mainLayout->addWidget(table_);

    // Connections
    connect(newProjectBtn_, &QPushButton::clicked, this, &ProjectsView::createProjectRequested);
    connect(searchBar_, &QLineEdit::textChanged, this, &ProjectsView::onSearchChanged);
    connect(statusFilter_, &QComboBox::currentIndexChanged, this, &ProjectsView::onStatusFilterChanged);
    connect(table_, &QTableWidget::cellDoubleClicked, this, &ProjectsView::onRowDoubleClicked);
}

void ProjectsView::refresh() {
    database::ProjectRepository repo;
    auto projects = repo.getAll();

    table_->setRowCount(0);

    QString searchVal = searchBar_->text().trimmed().toLower();
    QString statusVal = statusFilter_->currentText();

    int row = 0;
    for (const auto& p : projects) {
        QString customer = QString::fromStdString(p.customer);
        QString name = QString::fromStdString(p.name);
        QString status = QString::fromStdString(core::projectStatusToString(p.status));

        // Filters matching
        if (!searchVal.isEmpty() && !customer.toLower().contains(searchVal) && !name.toLower().contains(searchVal)) {
            continue;
        }
        if (statusFilter_->currentIndex() > 0 && status != statusVal) {
            continue;
        }

        table_->insertRow(row);
        
        auto* idItem = new QTableWidgetItem(QString::number(p.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        
        table_->setItem(row, 0, idItem);
        table_->setItem(row, 1, new QTableWidgetItem(customer));
        table_->setItem(row, 2, new QTableWidgetItem(name));
        table_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(p.startDate)));
        table_->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(p.endDate)));
        
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        
        // Severity-like colors for project status
        if (p.status == core::ProjectStatus::Planning) {
            statusItem->setForeground(QColor("#64748b")); // Grey
        } else if (p.status == core::ProjectStatus::Testing) {
            statusItem->setForeground(QColor("#fb923c")); // Orange
        } else if (p.status == core::ProjectStatus::Retesting) {
            statusItem->setForeground(QColor("#38bdf8")); // Blue
        } else if (p.status == core::ProjectStatus::Completed) {
            statusItem->setForeground(QColor("#4ade80")); // Green
        }
        table_->setItem(row, 5, statusItem);

        row++;
    }
}

void ProjectsView::onSearchChanged(const QString&) {
    refresh();
}

void ProjectsView::onStatusFilterChanged(int) {
    refresh();
}

void ProjectsView::onRowDoubleClicked(int row, int) {
    auto* idItem = table_->item(row, 0);
    if (idItem) {
        int id = idItem->text().toInt();
        emit projectSelected(id);
    }
}

} // namespace reportforge::widgets
