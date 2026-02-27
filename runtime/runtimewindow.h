#ifndef RUNTIMEWINDOW_H
#define RUNTIMEWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QDebug>
#include <QLayout>
#include "canvasitem.h"
#include "iconlabel.h"
class CanvasView;

namespace Ui {
class RuntimeWindow;
}

class RuntimeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RuntimeWindow(QWidget *parent = nullptr);
    ~RuntimeWindow();

    void setCanvas(CanvasView* canvas); // 设置需要显示的 CanvasView

protected:
    void resizeEvent(QResizeEvent* event);
    void setupIconButton(QPushButton* btn, const QString& iconPath);

private slots:
    void on_buttonOfQuit_clicked();

private:
    Ui::RuntimeWindow *ui;
    CanvasView* m_canvas = nullptr;


    QWidget* m_originalParent = nullptr;
    QLayout* m_originalLayout = nullptr;
    int m_originalIndex = -1;
    QSize m_originalSize;
    int m_originalStretch = 0;
    QSizePolicy m_originalPolicy;

     QMap<CanvasItem*, QRect> m_originalItemGeometries;

      QVector<int> m_originalStretchs; // 保存父布局所有 widget 的 stretch

};
#endif // RUNTIMEWINDOW_H
