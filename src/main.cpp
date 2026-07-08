#include <QApplication>
#include <QIcon>
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
#if defined(Q_OS_LINUX)
    // Force X11 (XWayland) backend on Linux.
    // Wayland's xdg-shell has no always-on-top protocol, so features like
    // WindowStaysOnTopHint only work reliably via X11/_NET_WM_STATE_ABOVE.
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/rhenocalc.svg"));
    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/rhenocalc.svg"));
    w.show();
    return QApplication::exec();
}
