#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QTextEdit>

namespace reportforge::widgets {

/**
 * @brief Custom widget with Editor and Preview tabs for Markdown editing.
 */
class MarkdownEditor : public QWidget {
    Q_OBJECT
public:
    explicit MarkdownEditor(QWidget* parent = nullptr);
    ~MarkdownEditor() override = default;

    /**
     * @brief Sets the text contents of the editor.
     */
    void setPlainText(const QString& text);

    /**
     * @brief Gets the plain text (raw markdown) contents from the editor.
     */
    QString toPlainText() const;

    /**
     * @brief Configures placeholder text inside the raw text editor.
     */
    void setPlaceholderText(const QString& text);

private slots:
    /**
     * @brief Triggered when user switches tabs; parses markdown to render the preview.
     */
    void onTabChanged(int index);

private:
    QTabWidget* tabWidget_;
    QTextEdit* editorText_;
    QTextEdit* previewText_;
};

} // namespace reportforge::widgets
