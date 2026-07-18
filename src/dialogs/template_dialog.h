#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include "../models/template.h"

namespace reportforge::dialogs {

/**
 * @brief Modal dialog to create or edit reusable vulnerability Templates.
 */
class TemplateDialog : public QDialog {
    Q_OBJECT
public:
    explicit TemplateDialog(QWidget* parent = nullptr);
    ~TemplateDialog() override = default;

    /**
     * @brief Loads an existing template's data into the form.
     */
    void setTemplate(const models::FindingTemplate& temp);

    /**
     * @brief Retrieves the modified or created template object.
     */
    models::FindingTemplate getTemplate() const;

private:
    int id_{0};
    QLineEdit* titleInput_;
    QComboBox* severityInput_;
    QDoubleSpinBox* cvssInput_;
    QLineEdit* cweInput_;
    QLineEdit* owaspInput_;
    QTextEdit* descInput_;
    QTextEdit* impactInput_;
    QTextEdit* recInput_;
};

} // namespace reportforge::dialogs
