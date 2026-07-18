#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

// Include repositories & managers
#include "database/db_manager.h"
#include "database/project_repository.h"
#include "database/template_repository.h"
#include "database/timeline_repository.h"

// Include widgets
#include "widgets/sidebar.h"
#include "widgets/dashboard_view.h"
#include "widgets/projects_view.h"
#include "widgets/project_detail_view.h"
#include "widgets/findings_view.h"
#include "widgets/finding_detail_view.h"
#include "widgets/templates_view.h"

// Include dialogs
#include "dialogs/project_dialog.h"
#include "dialogs/template_dialog.h"

using namespace reportforge;

/**
 * @brief Main Window class orchestrating the views stack and navigation.
 */
class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("ReportForge - Penetration Testing Report Manager");
        resize(1200, 800);

        auto* central = new QWidget(this);
        auto* mainLayout = new QHBoxLayout(central);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // 1. Sidebar navigation
        sidebar_ = new widgets::Sidebar(this);
        mainLayout->addWidget(sidebar_);

        // 2. Stacked workspace area
        stackedWidget_ = new QStackedWidget(this);
        
        dashboard_ = new widgets::DashboardView(this);
        projects_ = new widgets::ProjectsView(this);
        findings_ = new widgets::FindingsView(this);
        templates_ = new widgets::TemplatesView(this);
        projectDetail_ = new widgets::ProjectDetailView(this);
        findingDetail_ = new widgets::FindingDetailView(this);

        stackedWidget_->addWidget(dashboard_);      // Index 0
        stackedWidget_->addWidget(projects_);       // Index 1
        stackedWidget_->addWidget(findings_);       // Index 2
        stackedWidget_->addWidget(templates_);      // Index 3
        stackedWidget_->addWidget(projectDetail_);   // Index 4
        stackedWidget_->addWidget(findingDetail_);   // Index 5

        mainLayout->addWidget(stackedWidget_, 1);
        setCentralWidget(central);

        setupConnections();
        
        // Load initial data
        dashboard_->refresh();
    }

private:
    void setupConnections() {
        // Sidebar navigation transitions
        connect(sidebar_, &widgets::Sidebar::navigationChanged, this, [this](int index) {
            // Refresh views before switching
            if (index == 0) dashboard_->refresh();
            else if (index == 1) projects_->refresh();
            else if (index == 2) findings_->refresh();
            else if (index == 3) templates_->refresh();
            
            stackedWidget_->setCurrentIndex(index);
        });

        // Navigation from Dashboard and Projects View to Project Detail Workspace
        connect(dashboard_, &widgets::DashboardView::projectSelected, this, &MainWindow::openProjectDetail);
        connect(projects_, &widgets::ProjectsView::projectSelected, this, &MainWindow::openProjectDetail);

        // Project creation transition
        connect(projects_, &widgets::ProjectsView::createProjectRequested, this, &MainWindow::onCreateProject);

        // Navigation inside Project detail workspace
        connect(projectDetail_, &widgets::ProjectDetailView::backRequested, this, [this]() {
            projects_->refresh();
            stackedWidget_->setCurrentIndex(1); // Back to projects list
        });
        connect(projectDetail_, &widgets::ProjectDetailView::editProjectRequested, this, &MainWindow::onEditProject);

        // Findings management transitions inside Project Detail
        connect(projectDetail_, &widgets::ProjectDetailView::createFindingRequested, this, [this](int projectId) {
            findingDetail_->setFinding(0, projectId);
            stackedWidget_->setCurrentIndex(5); // Switch to finding editor
        });
        connect(projectDetail_, &widgets::ProjectDetailView::editFindingRequested, this, [this](int findingId) {
            findingDetail_->setFinding(findingId);
            stackedWidget_->setCurrentIndex(5);
        });

        // Findings Global View transition
        connect(findings_, &widgets::FindingsView::editFindingRequested, this, [this](int findingId) {
            findingDetail_->setFinding(findingId);
            stackedWidget_->setCurrentIndex(5);
        });

        // Finding Editor transitions
        connect(findingDetail_, &widgets::FindingDetailView::backRequested, this, [this](int projectId) {
            projectDetail_->setProject(projectId);
            stackedWidget_->setCurrentIndex(4); // Back to project workspace
        });

        // Templates View transitions
        connect(templates_, &widgets::TemplatesView::createTemplateRequested, this, &MainWindow::onCreateTemplate);
        connect(templates_, &widgets::TemplatesView::editTemplateRequested, this, &MainWindow::onEditTemplate);
    }

    void openProjectDetail(int projectId) {
        projectDetail_->setProject(projectId);
        stackedWidget_->setCurrentIndex(4); // Switch to project detail workspace
    }

    void onCreateProject() {
        dialogs::ProjectDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            auto newProj = dlg.getProject();
            database::ProjectRepository repo;
            if (repo.insert(newProj)) {
                int newId = repo.getLastInsertedId();
                
                // Log initialization event
                database::TimelineRepository timeRepo;
                timeRepo.logEvent(newId, "Project created.");
                
                openProjectDetail(newId);
            } else {
                QMessageBox::critical(this, "Error", "Failed to create project in database.");
            }
        }
    }

    void onEditProject(int projectId) {
        database::ProjectRepository repo;
        auto optProj = repo.getById(projectId);
        if (!optProj.has_value()) return;

        dialogs::ProjectDialog dlg(this);
        dlg.setProject(optProj.value());
        if (dlg.exec() == QDialog::Accepted) {
            auto updatedProj = dlg.getProject();
            if (repo.update(updatedProj)) {
                projectDetail_->refresh();
            } else {
                QMessageBox::critical(this, "Error", "Failed to update project data.");
            }
        }
    }

    void onCreateTemplate() {
        dialogs::TemplateDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            auto t = dlg.getTemplate();
            database::TemplateRepository repo;
            if (repo.insert(t)) {
                templates_->refresh();
            } else {
                QMessageBox::critical(this, "Error", "Failed to save template.");
            }
        }
    }

    void onEditTemplate(int templateId) {
        database::TemplateRepository repo;
        auto optT = repo.getById(templateId);
        if (!optT.has_value()) return;

        dialogs::TemplateDialog dlg(this);
        dlg.setTemplate(optT.value());
        if (dlg.exec() == QDialog::Accepted) {
            auto t = dlg.getTemplate();
            if (repo.update(t)) {
                templates_->refresh();
            } else {
                QMessageBox::critical(this, "Error", "Failed to update template.");
            }
        }
    }

private:
    widgets::Sidebar* sidebar_;
    QStackedWidget* stackedWidget_;

    widgets::DashboardView* dashboard_;
    widgets::ProjectsView* projects_;
    widgets::FindingsView* findings_;
    widgets::TemplatesView* templates_;
    widgets::ProjectDetailView* projectDetail_;
    widgets::FindingDetailView* findingDetail_;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Initialize Database
    if (!database::DbManager::instance().initialize("reportforge.db")) {
        return 1;
    }

    // Apply Premium Cyber Security Dark Theme stylesheet globally
    app.setStyleSheet(
        "QWidget {"
        "  background-color: #0c0c0e;"
        "  color: #e2e8f0;"
        "  font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QMainWindow {"
        "  background-color: #0c0c0e;"
        "}"
        "QLabel {"
        "  color: #e2e8f0;"
        "}"
        "QLineEdit, QTextEdit, QPlainTextEdit, QDateEdit, QDoubleSpinBox, QSpinBox {"
        "  background-color: #16161e;"
        "  color: #e2e8f0;"
        "  border: 1px solid #23232f;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  selection-background-color: #8b5cf6;"
        "}"
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QDateEdit:focus, QDoubleSpinBox:focus, QSpinBox:focus {"
        "  border: 1px solid #8b5cf6;"
        "}"
        "QPushButton {"
        "  background-color: #1a1a24;"
        "  color: #e2e8f0;"
        "  border: 1px solid #2d2d3c;"
        "  border-radius: 6px;"
        "  padding: 6px 14px;"
        "  font-size: 11px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background-color: #232332;"
        "  border-color: #8b5cf6;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1a1a24;"
        "}"
        "QScrollBar:vertical {"
        "  border: none;"
        "  background: #111115;"
        "  width: 10px;"
        "  margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #2d2d3c;"
        "  min-height: 20px;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #8b5cf6;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  border: none;"
        "  background: none;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
