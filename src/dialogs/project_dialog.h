#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QComboBox>
#include "../models/project.h"

namespace reportforge::dialogs {

/**
 * @brief Modal dialog to create or edit basic Project metadata.
 */
class ProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProjectDialog(QWidget* parent = nullptr);
    ~ProjectDialog() override = default;

    /**
     * @brief Loads an existing project's data into the form.
     */
    void setProject(const models::Project& project);

    /**
     * @brief Retrieves the modified or created project object.
     */
    models::Project getProject() const;

private:
    int projectId_{0};
    QLineEdit* customerInput_;
    QLineEdit* nameInput_;
    QTextEdit* descInput_;
    QDateEdit* startInput_;
    QDateEdit* endInput_;
    QComboBox* statusInput_;

    // Preserved fields that aren't managed in this dialog
    std::string scope_;
    std::string targets_;
    std::string notes_;
};

} // namespace reportforge::dialogs
