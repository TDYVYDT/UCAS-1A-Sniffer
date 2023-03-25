#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QObject::connect(&a, &QApplication::aboutToQuit, [&]() {
       w.close_sniffer();
    });
    return a.exec();
}
