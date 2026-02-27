#include "virtualkeyboardhost.h".h"

VirtualKeyboardHost::VirtualKeyboardHost(QWidget *parent)
    : QQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setClearColor(Qt::transparent);

    // 加载 Qt 自带键盘 QML
    setSource(QUrl("qrc:/qt-project.org/imports/QtQuick/VirtualKeyboard/content/InputPanel.qml"));

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
}

