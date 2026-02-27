// fullscreenview.h
#ifndef FULLSCREENVIEW_H
#define FULLSCREENVIEW_H

#include <QWidget>
#include <QPushButton>
#include "canvas/canvasview.h"

class FullScreenView : public QWidget {
    Q_OBJECT
public:
    explicit FullScreenView(CanvasView* canvas, QWidget *parent = nullptr);

signals:
    void exitFullScreen();

private:
    CanvasView* m_canvas;
    QPushButton* m_exitBtn;
};

#endif // FULLSCREENVIEW_H
