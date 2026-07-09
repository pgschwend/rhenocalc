#include "bitbutton.h"
#include "ui/themecolors.h"

BitButton::BitButton(int bitIndex, QWidget* parent)
    : QPushButton(parent), m_bitIndex(bitIndex) {
    setFixedSize(14, 14);
    setCheckable(false);
    refresh();
    connect(this, &QPushButton::clicked, this, [this]{
        m_state = !m_state;
        refresh();
        emit toggled2(m_bitIndex, m_state);
    });
}

void BitButton::setState(bool on) {
    m_state = on;
    refresh();
}

void BitButton::setDark(bool dark) {
    m_dark = dark;
    refresh();
}

void BitButton::refresh() {
    setText(m_state ? "1" : "0");
    setStyleSheet(Rheno::UI::baseBitButtonStyle(m_dark, m_state));
}

