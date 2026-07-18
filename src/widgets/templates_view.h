#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

namespace reportforge::widgets {

/**
 * @brief View for managing reusable Finding Templates.
 */
class TemplatesView : public QWidget {
    Q_OBJECT
public:
    explicit TemplatesView(QWidget* parent = nullptr);
    ~TemplatesView() override = default;

    /**
     * @brief Reloads the templates list from the database.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when a template is selected for editing.
     */
    void editTemplateRequested(int templateId);

    /**
     * @brief Emitted to request creating a new template.
     */
    void createTemplateRequested();

private slots:
    void onRowDoubleClicked(int row, int col);
    void onDeleteTemplate();

private:
    QTableWidget* table_;
    QPushButton* addTemplateBtn_;
    QPushButton* deleteTemplateBtn_;
};

} // namespace reportforge::widgets
