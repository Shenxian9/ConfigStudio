#include "mainwindow.h"
#include "canvas/canvasview.h"
#include <QApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QPixmap>
#include <QPainter>
#include <QScreen>

#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>



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
