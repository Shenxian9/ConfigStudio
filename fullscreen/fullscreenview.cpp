// fullscreenview.cpp
#include "fullscreenview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>

FullScreenView::FullScreenView(CanvasView* canvas, QWidget *parent)
    : QWidget(parent), m_canvas(canvas)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // 无边框全屏
    setWindowState(Qt::WindowFullScreen);

    // 背景布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    // 顶部按钮布局
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->addStretch(); // 右上角
    m_exitBtn = new QPushButton("Exit", this);
    topLayout->addWidget(m_exitBtn);

    mainLayout->addLayout(topLayout);

    // 将 CanvasView 放到全屏窗口
    m_canvas->setParent(this);
    m_canvas->show();

    mainLayout->addWidget(m_canvas);

    connect(m_exitBtn, &QPushButton::clicked, this, &FullScreenView::exitFullScreen);
}
