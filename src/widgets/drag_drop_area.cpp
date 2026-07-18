#include "drag_drop_area.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QVBoxLayout>
#include <QFileInfo>

namespace reportforge::widgets {

DragDropArea::DragDropArea(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setMinimumHeight(100);
    setFrameStyle(QFrame::StyledPanel);

    // Initial cyber styling (dashed border with purple accent)
    setObjectName("DragDropArea");
    setStyleSheet(
        "QFrame#DragDropArea {"
        "  border: 2px dashed #8b5cf6;"
        "  border-radius: 8px;"
        "  background-color: rgba(139, 92, 246, 0.05);"
        "}"
    );

    auto* layout = new QVBoxLayout(this);
    label_ = new QLabel("Drag & Drop Evidence Files Here\n(PNG, JPG, PDF, TXT, LOG)", this);
    label_->setAlignment(Qt::AlignCenter);
    label_->setStyleSheet(
        "color: #a78bfa;"
        "font-weight: bold;"
        "font-size: 11px;"
    );
    layout->addWidget(label_);
}

void DragDropArea::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        // Hover cyber styling (dashed border with neon blue accent)
        setStyleSheet(
            "QFrame#DragDropArea {"
            "  border: 2px dashed #06b6d4;"
            "  border-radius: 8px;"
            "  background-color: rgba(6, 182, 212, 0.1);"
            "}"
        );
        label_->setStyleSheet(
            "color: #22d3ee;"
            "font-weight: bold;"
            "font-size: 11px;"
        );
    }
}

void DragDropArea::dragLeaveEvent(QDragLeaveEvent* event) {
    Q_UNUSED(event);
    // Reset styling
    setStyleSheet(
        "QFrame#DragDropArea {"
        "  border: 2px dashed #8b5cf6;"
        "  border-radius: 8px;"
        "  background-color: rgba(139, 92, 246, 0.05);"
        "}"
    );
    label_->setStyleSheet(
        "color: #a78bfa;"
        "font-weight: bold;"
        "font-size: 11px;"
    );
}

void DragDropArea::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList filePaths;
        for (const QUrl& url : mimeData->urls()) {
            QString localPath = url.toLocalFile();
            if (!localPath.isEmpty()) {
                filePaths.append(localPath);
            }
        }
        if (!filePaths.isEmpty()) {
            emit filesDropped(filePaths);
        }
    }
    // Reset styling after drop
    dragLeaveEvent(nullptr);
}

} // namespace reportforge::widgets
