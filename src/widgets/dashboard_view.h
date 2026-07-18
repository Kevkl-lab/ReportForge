#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>

namespace reportforge::widgets {

/**
 * @brief Dashboard widget presenting aggregate statistics, findings distribution, and recent activity.
 */
class DashboardView : public QWidget {
    Q_OBJECT
public:
    explicit DashboardView(QWidget* parent = nullptr);
    ~DashboardView() override = default;

    /**
     * @brief Reloads and refreshes all statistics and recent project lists.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when a project card is clicked on the dashboard.
     */
    void projectSelected(int projectId);

private:
    QLabel* totalProjectsVal_;
    QLabel* criticalFindingsVal_;
    QLabel* openFindingsVal_;
    QLabel* totalFindingsVal_;
    QListWidget* recentProjectsList_;
};

} // namespace reportforge::widgets
