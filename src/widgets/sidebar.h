#pragma once

#include <QWidget>
#include <QButtonGroup>

namespace reportforge::widgets {

/**
 * @brief Left sidebar navigation panel for the main workspace shell.
 */
class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);
    ~Sidebar() override = default;

signals:
    /**
     * @brief Emitted when a sidebar navigation button is clicked.
     * @param viewIndex Integer representing the stack index to switch to.
     */
    void navigationChanged(int viewIndex);

private:
    QButtonGroup* buttonGroup_;
};

} // namespace reportforge::widgets
