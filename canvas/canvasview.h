#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#pragma once
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QList>

#include "canvasitem.h"
#include "runtime/databindingmanager.h"
class CanvasView : public QWidget {
    Q_OBJECT
public:
    explicit CanvasView(QWidget *parent = nullptr);
    explicit CanvasView(DataBindingManager* bindingMgr, QWidget *parent = nullptr);
    CanvasItem* addItem(const QString& type, const QRect& geometry, const QString& itemId = QString());
    QList<CanvasItem*> items() const;
    void clearSelection();
    void setScale(double s) { m_scale = s; update(); }

    void setBindingManager(DataBindingManager* mgr);
    //QSize designSize() const { return m_designSize; }

signals:
    void itemSelected(CanvasItem*);

    void emptyAreaClicked();



protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    //void resizeEvent(QResizeEvent* event) override;

private:
    void hookupItem(CanvasItem *item);
    CanvasItem *m_selected = nullptr;
    double m_scale = 1.0;
    bool m_scaleEnabled = false;              // 是否启用运行态等比缩放
    QSize m_canvasViewOriginalSize;  // 记录 CanvasView 原始大小，用于等比缩放

    DataBindingManager* m_bindingMgr = nullptr;

};


#endif // CANVASVIEW_H
