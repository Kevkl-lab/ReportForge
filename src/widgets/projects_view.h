#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

namespace reportforge::widgets {

/**
 * @brief View displaying the list of all penetration testing projects.
 */
class ProjectsView : public QWidget {
    Q_OBJECT
public:
    explicit ProjectsView(QWidget* parent = nullptr);
    ~ProjectsView() override = default;

    /**
     * @brief Reloads and populates the projects list, applying active filters.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when a project is selected (double clicked) from the table.
     */
    void projectSelected(int projectId);

    /**
     * @brief Emitted when the "New Project" button is clicked.
     */
    void createProjectRequested();

private slots:
    void onSearchChanged(const QString& text);
    void onStatusFilterChanged(int index);
    void onRowDoubleClicked(int row, int col);

private:
    QLineEdit* searchBar_;
    QComboBox* statusFilter_;
    QTableWidget* table_;
    QPushButton* newProjectBtn_;
};

} // namespace reportforge::widgets
