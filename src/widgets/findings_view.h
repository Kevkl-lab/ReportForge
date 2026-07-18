#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>

namespace reportforge::widgets {

/**
 * @brief View displaying all logged findings across all active projects.
 */
class FindingsView : public QWidget {
    Q_OBJECT
public:
    explicit FindingsView(QWidget* parent = nullptr);
    ~FindingsView() override = default;

    /**
     * @brief Reloads finding list from database and applies search filters.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when a finding row is double-clicked to view/edit it.
     */
    void editFindingRequested(int findingId);

private slots:
    void onSearchChanged(const QString& text);
    void onSeverityFilterChanged(int index);
    void onRowDoubleClicked(int row, int col);

private:
    QLineEdit* searchBar_;
    QComboBox* severityFilter_;
    QTableWidget* table_;
};

} // namespace reportforge::widgets
