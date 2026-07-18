#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTabWidget>
#include "markdown_editor.h"
#include "drag_drop_area.h"

namespace reportforge::widgets {

/**
 * @brief Rich editor view for adding or modifying vulnerability Findings.
 */
class FindingDetailView : public QWidget {
    Q_OBJECT
public:
    explicit FindingDetailView(QWidget* parent = nullptr);
    ~FindingDetailView() override = default;

    /**
     * @brief Loads a finding for editing.
     * @param findingId Database ID of the finding, or 0 if creating a new finding.
     * @param projectId Parent project ID (required if creating a new finding).
     */
    void setFinding(int findingId, int projectId = 0);

    /**
     * @brief Reloads data and updates form components.
     */
    void refresh();

signals:
    /**
     * @brief Emitted when saving or cancelling, pointing back to the project workspace.
     */
    void backRequested(int projectId);

private slots:
    void onSave();
    void onApplyTemplate(int index);
    void onFilesDropped(const QStringList& filePaths);
    void onDeleteEvidence();
    void onCvssMetricChanged();

private:
    int findingId_{0};
    int projectId_{0};

    // Metadata inputs
    QLineEdit* titleInput_;
    QComboBox* severityInput_;
    QDoubleSpinBox* cvssScoreInput_;
    QComboBox* templateSelect_;
    QLineEdit* owaspInput_;
    QLineEdit* cweInput_;
    QComboBox* statusInput_;
    QLineEdit* affectedAssetsInput_;
    QLineEdit* tagsInput_;

    // Tabbed Markdown editors
    QTabWidget* textTabs_;
    MarkdownEditor* descEditor_;
    MarkdownEditor* impactEditor_;
    MarkdownEditor* recEditor_;
    MarkdownEditor* pocEditor_;
    MarkdownEditor* stepsEditor_;

    // Evidence attachments
    DragDropArea* dragDropArea_;
    QListWidget* evidenceList_;

    // Built-in CVSS Calculator inputs
    QComboBox* avCombo_;
    QComboBox* acCombo_;
    QComboBox* prCombo_;
    QComboBox* uiCombo_;
    QComboBox* sCombo_;
    QComboBox* cCombo_;
    QComboBox* iCombo_;
    QComboBox* aCombo_;
    QLabel* cvssVectorLabel_;
};

} // namespace reportforge::widgets
