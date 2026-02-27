#ifndef VIRTUALKEYBOARDHOST_H
#define VIRTUALKEYBOARDHOST_H

#pragma once
#include <QQuickWidget>

class VirtualKeyboardHost : public QQuickWidget
{
    Q_OBJECT
public:
    explicit VirtualKeyboardHost(QWidget *parent = nullptr);
};

#endif // VIRTUALKEYBOARDHOST_H
