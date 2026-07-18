#pragma once

#include <QFrame>
#include <QLabel>
#include <QStringList>

namespace reportforge::widgets {

/**
 * @brief Custom drag & drop area supporting multiple file uploads for evidence.
 */
class DragDropArea : public QFrame {
    Q_OBJECT
public:
    explicit DragDropArea(QWidget* parent = nullptr);
    ~DragDropArea() override = default;

signals:
    /**
     * @brief Emitted when files are successfully dropped onto the area.
     */
    void filesDropped(const QStringList& filePaths);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QLabel* label_;
};

} // namespace reportforge::widgets
