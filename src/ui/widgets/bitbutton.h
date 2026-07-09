#pragma once

#include <QPushButton>

class BitButton : public QPushButton {
    Q_OBJECT
public:
    explicit BitButton(int bitIndex, QWidget* parent = nullptr);
    void setState(bool on);
    void setDark(bool dark);
    bool state() const { return m_state; }
    int  bitIndex() const { return m_bitIndex; }
signals:
    void toggled2(int bit, bool state);
private:
    void refresh();
    int  m_bitIndex;
    bool m_state = false;
    bool m_dark  = true;
};

