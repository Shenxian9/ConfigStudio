#include "ui/mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE",QByteArray("Qt5Input"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qDebug() << "IM module:" << qgetenv("QT_IM_MODULE");
    qDebug() << "Platform:" << QGuiApplication::platformName();
    qDebug() << "Input method object:" << qApp->inputMethod();
    return a.exec();
}
