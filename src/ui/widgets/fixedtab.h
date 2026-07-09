#pragma once

#include <QTabWidget>
#include <QTabBar>

// Custom tab bar with fixed tab widths
class FixedTabBar : public QTabBar {
public:
    using QTabBar::QTabBar;
    [[nodiscard]] QSize tabSizeHint(int index) const override {
        QSize s = QTabBar::tabSizeHint(index);
        if (index <= 1)       s.setWidth(58);   // Calc, Base
        else if (index == 2)  s.setWidth(86);   // dynamic tab
        else if (index == 3)  s.setWidth(34);   // "▾"
        return s;
    }
};

// QTabWidget that uses the FixedTabBar
class FixedTabWidget : public QTabWidget {
public:
    explicit FixedTabWidget(QWidget* parent = nullptr) : QTabWidget(parent) {
        setTabBar(new FixedTabBar(this)); // NOLINT
    }
};

