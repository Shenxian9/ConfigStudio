#include "canvasitem.h"
#include <QPainter>

CanvasItem::CanvasItem(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    if (m_editLocked) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    emit selected(this);

    if (isInResizeHandle(event->pos())) {
        m_resizing = true;
        m_dragStart = event->globalPos();
        m_startRect = rect();
    }
    else {
        m_dragging = true;
        m_dragStart = event->globalPos();
        m_startRect = geometry();
    }

    event->accept();            // ★ 关键：告诉 Qt 这个事件已被 item 消费
}



void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_editLocked) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint delta = event->globalPos() - m_dragStart;

    if (m_resizing) {
        int newWidth = std::max(20, m_startRect.width() + delta.x());
        int newHeight = std::max(20, m_startRect.height() + delta.y());
        resize(newWidth, newHeight);
    }
    else if (m_dragging) {
        move(m_startRect.topLeft() + delta);
    }

    QWidget::mouseMoveEvent(event);
}


void CanvasItem::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (m_selected) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QStyleOption opt;
        opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

        QPen pen(Qt::blue);
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine); // 虚线
        painter.setPen(pen);

        QRect r = rect();
        r.adjust(2,2,-2,-2); // 防止边框被裁剪
        painter.drawRect(r);
        QRect handle(rect().right()-handleSize, rect().bottom()-handleSize, handleSize, handleSize);
        painter.fillRect(handle, Qt::blue);
    }
}

bool CanvasItem::isInResizeHandle(const QPoint& pos) const
{
    QRect handle(rect().right()-handleSize, rect().bottom()-handleSize, handleSize, handleSize);
    return handle.contains(pos);
}
void CanvasItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_editLocked) {
        m_resizing = false;
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}
