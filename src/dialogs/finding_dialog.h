#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include "../models/finding.h"

namespace reportforge::dialogs {

/**
 * @brief Modal dialog for quick editing of Finding metadata.
 */
class FindingDialog : public QDialog {
    Q_OBJECT
public:
    explicit FindingDialog(QWidget* parent = nullptr);
    ~FindingDialog() override = default;

    /**
     * @brief Loads an existing finding's metadata into the form.
     */
    void setFinding(const models::Finding& finding);

    /**
     * @brief Retrieves the modified finding object.
     */
    models::Finding getFinding() const;

private:
    int id_{0};
    int projectId_{0};
    QLineEdit* titleInput_;
    QComboBox* severityInput_;
    QDoubleSpinBox* cvssInput_;
    QLineEdit* cweInput_;
    QLineEdit* owaspInput_;
    QComboBox* statusInput_;
    QLineEdit* assetsInput_;
    QLineEdit* tagsInput_;

    // Preserved fields that are managed in the main detail workspace
    std::string desc_;
    std::string impact_;
    std::string rec_;
    std::string poc_;
    std::string steps_;
    std::string evidence_;
};

} // namespace reportforge::dialogs
