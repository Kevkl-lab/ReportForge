#include "sidebar.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace reportforge::widgets {

Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    // Set fixed width for sidebar
    setFixedWidth(200);
    setStyleSheet(
        "QWidget {"
        "  background-color: #111115;"
        "  border-right: 1px solid #22222a;"
        "}"
    );

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 20, 0, 20);
    layout->setSpacing(5);

    // App Brand Logo/Header
    auto* brandLabel = new QLabel("REPORTFORGE", this);
    brandLabel->setAlignment(Qt::AlignCenter);
    brandLabel->setStyleSheet(
        "color: #a78bfa;"
        "font-weight: bold;"
        "font-size: 16px;"
        "letter-spacing: 2px;"
        "padding: 15px 0px;"
        "border: none;"
    );
    layout->addWidget(brandLabel);

    // Sidebar navigation buttons definitions
    struct NavItem {
        QString text;
        int index;
    };
    std::vector<NavItem> navItems = {
        {"Dashboard", 0},
        {"Projects", 1},
        {"Findings", 2},
        {"Templates", 3},
        {"Reports", 4},
        {"Settings", 5}
    };

    buttonGroup_ = new QButtonGroup(this);
    buttonGroup_->setExclusive(true);

    for (const auto& item : navItems) {
        auto* btn = new QPushButton(item.text, this);
        btn->setCheckable(true);
        btn->setMinimumHeight(45);
        btn->setStyleSheet(
            "QPushButton {"
            "  text-align: left;"
            "  padding-left: 20px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #94a3b8;"
            "  background-color: transparent;"
            "  border: none;"
            "  border-left: 4px solid transparent;"
            "}"
            "QPushButton:hover {"
            "  color: #e2e8f0;"
            "  background-color: #1a1a24;"
            "}"
            "QPushButton:checked {"
            "  color: #a78bfa;"
            "  background-color: #1a1a24;"
            "  border-left: 4px solid #8b5cf6;"
            "  font-weight: bold;"
            "}"
        );

        buttonGroup_->addButton(btn, item.index);
        layout->addWidget(btn);
    }

    // Set first item checked by default
    if (auto* firstBtn = qobject_cast<QPushButton*>(buttonGroup_->button(0))) {
        firstBtn->setChecked(true);
    }

    layout->addStretch();

    // Footer
    auto* footerLabel = new QLabel("v1.0.0", this);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setStyleSheet(
        "color: #4b5563;"
        "font-size: 10px;"
        "border: none;"
    );
    layout->addWidget(footerLabel);

    // Forward click events as signals
    connect(buttonGroup_, &QButtonGroup::idClicked, this, &Sidebar::navigationChanged);
}

} // namespace reportforge::widgets
