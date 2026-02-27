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
    // 指定 Qt 输入法模块，确保在触控/虚拟键盘环境下可正常调起输入法。
    qputenv("QT_IM_MODULE",QByteArray("Qt5Input"));



    // 初始化应用对象并显示主窗口。
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // 启动时输出关键环境信息，便于排查输入法或平台适配问题。
    qDebug() << "IM module:" << qgetenv("QT_IM_MODULE");
    qDebug() << "Platform:" << QGuiApplication::platformName();
    qDebug() << "Input method object:" << qApp->inputMethod();


    // 进入 Qt 事件循环。
    return a.exec();
}
