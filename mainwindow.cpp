#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 由 Qt Designer 生成的界面初始化入口。
    // 该调用会创建 .ui 中定义的控件层级并完成信号槽自动连接。
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    // 释放界面对象，避免内存泄漏。
    delete ui;
}
