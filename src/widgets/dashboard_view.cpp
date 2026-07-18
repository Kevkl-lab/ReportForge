#include "dashboard_view.h"
#include "../database/project_repository.h"
#include "../database/finding_repository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>

namespace reportforge::widgets {

static QFrame* createStatCard(const QString& title, QLabel*& valueLabel, const QString& colorHex) {
    auto* card = new QFrame();
    card->setObjectName("StatCard");
    card->setStyleSheet(
        QString(
            "QFrame#StatCard {"
            "  background-color: #16161e;"
            "  border: 1px solid #23232f;"
            "  border-radius: 10px;"
            "  padding: 15px;"
            "}"
        )
    );

    auto* layout = new QVBoxLayout(card);
    layout->setSpacing(5);

    auto* titleLabel = new QLabel(title.toUpper());
    titleLabel->setStyleSheet("color: #64748b; font-size: 10px; font-weight: bold; letter-spacing: 1px;");
    layout->addWidget(titleLabel);

    valueLabel = new QLabel("0");
    valueLabel->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold;").arg(colorHex));
    layout->addWidget(valueLabel);

    return card;
}

DashboardView::DashboardView(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(25);

    // Page Title
    auto* titleLabel = new QLabel("DASHBOARD", this);
    titleLabel->setStyleSheet("color: #e2e8f0; font-size: 20px; font-weight: bold; letter-spacing: 1px;");
    mainLayout->addWidget(titleLabel);

    // Horizontal layout for stat cards
    auto* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);

    statsLayout->addWidget(createStatCard("Total Projects", totalProjectsVal_, "#a78bfa"));       // Purple
    statsLayout->addWidget(createStatCard("Critical Findings", criticalFindingsVal_, "#f87171")); // Light Red
    statsLayout->addWidget(createStatCard("Open Findings", openFindingsVal_, "#fb923c"));         // Orange
    statsLayout->addWidget(createStatCard("Total Findings", totalFindingsVal_, "#38bdf8"));       // Light Blue

    mainLayout->addLayout(statsLayout);

    // Recently Modified projects section
    auto* bottomLayout = new QVBoxLayout();
    bottomLayout->setSpacing(10);

    auto* sectionTitle = new QLabel("RECENTLY MODIFIED PROJECTS", this);
    sectionTitle->setStyleSheet("color: #64748b; font-size: 11px; font-weight: bold; letter-spacing: 1px;");
    bottomLayout->addWidget(sectionTitle);

    recentProjectsList_ = new QListWidget(this);
    recentProjectsList_->setStyleSheet(
        "QListWidget {"
        "  background-color: #16161e;"
        "  border: 1px solid #23232f;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  color: #e2e8f0;"
        "}"
        "QListWidget::item {"
        "  padding: 12px 15px;"
        "  border-bottom: 1px solid #1f1f2a;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #1a1a24;"
        "  color: #a78bfa;"
        "  border-radius: 4px;"
        "}"
    );
    bottomLayout->addWidget(recentProjectsList_);

    mainLayout->addLayout(bottomLayout, 1);

    // List item click connection
    connect(recentProjectsList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int projectId = item->data(Qt::UserRole).toInt();
        emit projectSelected(projectId);
    });
}

void DashboardView::refresh() {
    database::ProjectRepository projRepo;
    database::FindingRepository findRepo;

    auto projects = projRepo.getAll();
    auto findings = findRepo.getAll();

    // Set stat numbers
    totalProjectsVal_->setText(QString::number(projects.size()));
    criticalFindingsVal_->setText(QString::number(findRepo.getCountBySeverity(core::Severity::Critical)));
    openFindingsVal_->setText(QString::number(findRepo.getCountByStatus(core::FindingStatus::Open)));
    totalFindingsVal_->setText(QString::number(findings.size()));

    // Populate recent projects list (up to 5 recent)
    recentProjectsList_->clear();
    int count = 0;
    for (const auto& p : projects) {
        if (count >= 5) break;
        
        auto* item = new QListWidgetItem(recentProjectsList_);
        QString statusStr = QString::fromStdString(core::projectStatusToString(p.status)).toUpper();
        QString itemText = QString("%1  -  %2 (%3)  [Dates: %4 to %5]")
                               .arg(QString::fromStdString(p.customer))
                               .arg(QString::fromStdString(p.name))
                               .arg(statusStr)
                               .arg(QString::fromStdString(p.startDate))
                               .arg(QString::fromStdString(p.endDate));

        item->setText(itemText);
        item->setData(Qt::UserRole, p.id);
        recentProjectsList_->addItem(item);
        count++;
    }
}

} // namespace reportforge::widgets
