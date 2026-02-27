#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 由 Qt Designer 生成的界面初始化入口。
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    // 释放界面对象，避免内存泄漏。
    delete ui;
}
