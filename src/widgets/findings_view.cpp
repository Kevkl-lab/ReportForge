#include "findings_view.h"
#include "../database/finding_repository.h"
#include "../database/project_repository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVariant>
#include <QDebug>
#include <map>

namespace reportforge::widgets {

FindingsView::FindingsView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Header row
    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("ALL DISCOVERED FINDINGS", this);
    titleLabel->setStyleSheet("color: #e2e8f0; font-size: 20px; font-weight: bold; letter-spacing: 1px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Filters row
    auto* filtersLayout = new QHBoxLayout();
    filtersLayout->setSpacing(15);

    searchBar_ = new QLineEdit(this);
    searchBar_->setPlaceholderText("Search by Title, CWE or OWASP Category...");
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

    severityFilter_ = new QComboBox(this);
    severityFilter_->addItem("All Severities");
    severityFilter_->addItem("Critical");
    severityFilter_->addItem("High");
    severityFilter_->addItem("Medium");
    severityFilter_->addItem("Low");
    severityFilter_->addItem("Informational");
    severityFilter_->setMinimumHeight(35);
    severityFilter_->setFixedWidth(150);
    severityFilter_->setStyleSheet(
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
    filtersLayout->addWidget(severityFilter_);
    mainLayout->addLayout(filtersLayout);

    // Table setup
    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({"ID", "Project", "Finding Title", "Severity", "CVSS Score", "Status"});
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
        "QTableWidget::item {"
        "  padding: 10px;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #232332;"
        "  color: #a78bfa;"
        "}"
    );
    
    table_->setAlternatingRowColors(true);
    mainLayout->addWidget(table_);

    // Connections
    connect(searchBar_, &QLineEdit::textChanged, this, &FindingsView::onSearchChanged);
    connect(severityFilter_, &QComboBox::currentIndexChanged, this, &FindingsView::onSeverityFilterChanged);
    connect(table_, &QTableWidget::cellDoubleClicked, this, &FindingsView::onRowDoubleClicked);
}

void FindingsView::refresh() {
    database::FindingRepository findRepo;
    database::ProjectRepository projRepo;

    auto findings = findRepo.getAll();
    auto projects = projRepo.getAll();

    // Cache project names
    std::map<int, QString> projNames;
    for (const auto& p : projects) {
        projNames[p.id] = QString::fromStdString(p.name);
    }

    table_->setRowCount(0);

    QString searchVal = searchBar_->text().trimmed().toLower();
    QString sevFilter = severityFilter_->currentText();

    int row = 0;
    for (const auto& f : findings) {
        QString title = QString::fromStdString(f.title);
        QString sevStr = QString::fromStdString(core::severityToString(f.severity));
        QString cweStr = QString::fromStdString(f.cwe);
        QString owaspStr = QString::fromStdString(f.owaspCategory);

        // Apply filters
        if (!searchVal.isEmpty() && 
            !title.toLower().contains(searchVal) && 
            !cweStr.toLower().contains(searchVal) && 
            !owaspStr.toLower().contains(searchVal)) {
            continue;
        }

        if (severityFilter_->currentIndex() > 0 && sevStr != sevFilter) {
            continue;
        }

        table_->insertRow(row);

        auto* idItem = new QTableWidgetItem(QString::number(f.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 0, idItem);

        // Project column
        QString projName = projNames.count(f.projectId) ? projNames[f.projectId] : "Unknown Project";
        table_->setItem(row, 1, new QTableWidgetItem(projName));

        // Title column
        table_->setItem(row, 2, new QTableWidgetItem(title));

        // Severity column
        auto* sevItem = new QTableWidgetItem(sevStr);
        sevItem->setTextAlignment(Qt::AlignCenter);
        if (f.severity == core::Severity::Critical) {
            sevItem->setForeground(QColor("#f87171")); // Red
        } else if (f.severity == core::Severity::High) {
            sevItem->setForeground(QColor("#fb923c")); // Orange
        } else if (f.severity == core::Severity::Medium) {
            sevItem->setForeground(QColor("#fbbf24")); // Yellow
        } else if (f.severity == core::Severity::Low) {
            sevItem->setForeground(QColor("#60a5fa")); // Blue
        } else {
            sevItem->setForeground(QColor("#94a3b8")); // Grey
        }
        table_->setItem(row, 3, sevItem);

        // CVSS score column
        auto* cvssItem = new QTableWidgetItem(QString::number(f.cvssScore, 'f', 1));
        cvssItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 4, cvssItem);

        // Status column
        auto* statusItem = new QTableWidgetItem(QString::fromStdString(core::findingStatusToString(f.status)));
        statusItem->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 5, statusItem);

        row++;
    }
}

void FindingsView::onSearchChanged(const QString&) {
    refresh();
}

void FindingsView::onSeverityFilterChanged(int) {
    refresh();
}

void FindingsView::onRowDoubleClicked(int row, int) {
    auto* idItem = table_->item(row, 0);
    if (idItem) {
        int id = idItem->text().toInt();
        emit editFindingRequested(id);
    }
}

} // namespace reportforge::widgets
