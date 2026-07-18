#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>
#include "markdown_editor.h"

namespace reportforge::widgets {

/**
 * @brief Detailed management workspace for a single selected Project.
 */
class ProjectDetailView : public QWidget {
    Q_OBJECT
public:
    explicit ProjectDetailView(QWidget* parent = nullptr);
    ~ProjectDetailView() override = default;

    /**
     * @brief Loads a project by ID and refreshes the tab contents.
     */
    void setProject(int projectId);

    /**
     * @brief Reloads active project data from database and refreshes all views.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when clicking the "Back to Projects" button.
     */
    void backRequested();

    /**
     * @brief Emitted to open the edit project dialog.
     */
    void editProjectRequested(int projectId);

    /**
     * @brief Emitted when adding a finding under this project.
     */
    void createFindingRequested(int projectId);

    /**
     * @brief Emitted to open the edit finding editor.
     */
    void editFindingRequested(int findingId);

private slots:
    void onSaveNotes();
    void onSaveScopeAndTargets();
    void onAddTimelineEvent();
    void onGeneratePdf();
    void onExportJson();
    void onImportJson();
    void onFindingDoubleClicked(int row, int col);
    void onDeleteProject();

private:
    int projectId_{0};

    // Metadata summary headers
    QLabel* projNameLabel_;
    QLabel* custLabel_;
    QLabel* datesLabel_;
    QLabel* statusLabel_;

    // Tab contents
    QTabWidget* tabWidget_;

    // Scope tab editors
    MarkdownEditor* scopeEditor_;
    MarkdownEditor* targetsEditor_;

    // Notes tab editor
    MarkdownEditor* notesEditor_;

    // Findings tab table
    QTableWidget* findingsTable_;
    QPushButton* addFindingBtn_;

    // Timeline tab widgets
    QListWidget* timelineList_;
    QLineEdit* newEventInput_;
    QPushButton* addEventBtn_;

    // Report generation outputs
    QLineEdit* pdfPathInput_;
    QLineEdit* jsonPathInput_;
};

} // namespace reportforge::widgets
