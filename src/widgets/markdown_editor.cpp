#include "markdown_editor.h"
#include <QVBoxLayout>
#include <QTabBar>

namespace reportforge::widgets {

MarkdownEditor::MarkdownEditor(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    tabWidget_ = new QTabWidget(this);
    tabWidget_->setStyleSheet(
        "QTabWidget::pane {"
        "  border: 1px solid #2d2d34;"
        "  background: #1a1a1e;"
        "  border-radius: 6px;"
        "}"
        "QTabBar::tab {"
        "  background: #232329;"
        "  color: #94a3b8;"
        "  padding: 6px 12px;"
        "  margin-right: 2px;"
        "  border-top-left-radius: 4px;"
        "  border-top-right-radius: 4px;"
        "  border: 1px solid #2d2d34;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #1a1a1e;"
        "  color: #a78bfa;"
        "  border-bottom: 2px solid #8b5cf6;"
        "}"
    );

    // Editor tab
    editorText_ = new QTextEdit(this);
    editorText_->setFont(QFont("Courier New", 10));
    editorText_->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1a1a1e;"
        "  color: #e2e8f0;"
        "  border: none;"
        "  padding: 8px;"
        "}"
    );
    tabWidget_->addTab(editorText_, "Write");

    // Preview tab
    previewText_ = new QTextEdit(this);
    previewText_->setReadOnly(true);
    previewText_->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1a1a1e;"
        "  color: #e2e8f0;"
        "  border: none;"
        "  padding: 8px;"
        "}"
    );
    tabWidget_->addTab(previewText_, "Preview");

    layout->addWidget(tabWidget_);

    connect(tabWidget_, &QTabWidget::currentChanged, this, &MarkdownEditor::onTabChanged);
}

void MarkdownEditor::setPlainText(const QString& text) {
    editorText_->setPlainText(text);
    if (tabWidget_->currentIndex() == 1) {
        previewText_->setMarkdown(text);
    }
}

QString MarkdownEditor::toPlainText() const {
    return editorText_->toPlainText();
}

void MarkdownEditor::setPlaceholderText(const QString& text) {
    editorText_->setPlaceholderText(text);
}

void MarkdownEditor::onTabChanged(int index) {
    if (index == 1) {
        // Set Markdown parses syntax dynamically and displays styled output
        previewText_->setMarkdown(editorText_->toPlainText());
    }
}

} // namespace reportforge::widgets
