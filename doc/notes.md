
## Own UI-Framework with own classes (inherited from Q Object)

```cpp
class AlpWidget : public QWidget { ... };
class AlpButton : public QPushButton { ... };
class AlpCard : public QFrame { ... };
class AlpPage : public QWidget { ... };


// Maybe separated classes for different Objects

class AlpPrimaryButton : public AlpButton { ... };
class AlpOutlineButton : public AlpButton { ... };
class AlpDangerButton : public AlpButton { ... };



// And as a top feature, create an own theme

class AlpTheme {
public:
    static QColor primaryColor();
    static QColor accentColor();
    static QColor backgroundColor();
    static int cornerRadius();
    static QFont defaultFont();
};



// Example of own Object

class AlpPushButton : public QPushButton {
    Q_OBJECT
public:
    using QPushButton::QPushButton;
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        QColor bg = isDown() ? "#1565c0" : (underMouse() ? "#1e88e5" : "#1976d2");
        p.setBrush(bg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 6, 6);
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, text());
    }
};
```

