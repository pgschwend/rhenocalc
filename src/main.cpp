#include <QApplication>
#include <QIcon>
#include "ui/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/calculator.svg"));
    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/calculator.svg"));
    w.show();
    return QApplication::exec();
}
